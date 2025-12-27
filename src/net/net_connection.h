#ifndef INCLUDE_NET_CONNECTION_H
#define INCLUDE_NET_CONNECTION_H

#include "net_buffer.h"
#include "net_encoder.h"
#include "net_file_descriptor.h"
#include "net_utils.h"

#include <csignal>
#include <future>
#include <iostream>
#include <optional>
#include <sys/poll.h>
#include <sys/types.h>
#include <utility>

#define MAX_BUFFER_SIZE 4096
namespace net {

template <typename PROCESSOR, typename T>
concept Processor = requires(PROCESSOR & processor, T message)
{
    {processor.process(message)}->std::same_as<std::optional<T> >;
};

struct Signals {
    bool read;
    bool write;
    bool close;
};

template <Encoder ENCODER, ssize_t BUF_SIZE = MAX_BUFFER_SIZE>
class Connection {
    typedef ENCODER::MessageType MessageType;
    FileDescriptor               m_fd;

    Signals m_signals;

    Buffer m_incoming{BUF_SIZE};
    Buffer m_outgoing{BUF_SIZE};

    void read();
    void poll_incoming();

  public:
    Connection(FileDescriptor&& fd, Signals signals = {})
    : m_fd{std::move(fd)}
    , m_signals(signals)
    {
    }
    const FileDescriptor& fd() const { return m_fd; }

    bool want_read() const { return m_signals.read; }

    bool want_write() const { return m_signals.write; }

    bool want_close() const { return m_signals.close; }

    void send(MessageType message);

    std::future<typename ENCODER::MessageType> get_response();

    void write();

    template <typename PROCESSOR>
    requires Processor<PROCESSOR, MessageType> void
             process(PROCESSOR& processor);

    void close() { m_signals.close = true; }
};

template <Encoder ENCODER, ssize_t BUF_SIZE>
void Connection<ENCODER, BUF_SIZE>::poll_incoming()
{
    pollfd incoming_poll = {.fd = fd(), .events = POLL_IN | POLL_ERR};
    int    rc            = poll(&incoming_poll, 1, -1);
    if (rc < 0 && errno == EINTR) {
        return;  // not an error
    }
    utils::die_on(rc < 0, "unable to poll");
}

template <Encoder ENCODER, ssize_t BUF_SIZE>
void Connection<ENCODER, BUF_SIZE>::read()
{
    std::cout << "[SERVER][CONNECTION][READ] receiving server message "
                 "on connection fd: "
              << m_fd << std::endl;
    ssize_t rc = m_incoming.read_from(m_fd);
    if (rc < 0) {
        close();
        std::cout
            << "[SERVER][CONNECTION][READ] connection closed by peer fd: "
            << m_fd << std::endl;
        return;
    }
}

template <Encoder ENCODER, ssize_t BUF_SIZE>
void Connection<ENCODER, BUF_SIZE>::write()
{
    if (m_outgoing.size() == 0) {
        m_signals.read  = true;
        m_signals.write = false;
        return;
    }
    if (m_outgoing.write_to(m_fd, m_outgoing.size()) < 0) {
        if (errno != EAGAIN) {
            close();
            std::cout
                << "[SERVER][CONNECTION][WRITE] connection closed by peer fd: "
                << m_fd << std::endl;
            return;
        }
        std::cout << "[SERVER][CONNECTION][WRITE] EAGAIN on connection fd: "
                  << m_fd << std::endl;
    }
    else {
        m_outgoing.clear();
        std::cout << "[SERVER][CONNECTION][WRITE] sent server message on "
                     "connection fd: "
                  << m_fd << std::endl;
    }
}

template <Encoder ENCODER, ssize_t BUF_SIZE>
void Connection<ENCODER, BUF_SIZE>::send(MessageType message)
{
    ENCODER::write(message, m_outgoing);
    m_signals.write = true;
    m_signals.read  = false;
    write();
}

template <Encoder ENCODER, ssize_t BUF_SIZE>
std::future<typename ENCODER::MessageType>
Connection<ENCODER, BUF_SIZE>::get_response()
{
    return std::async([this]() {
        while (true) {
            read();
            if (std::optional<MessageType> message = ENCODER::consume_message(
                    m_incoming)) {
                return *message;
            }
        }
    });
}

template <Encoder ENCODER, ssize_t BUF_SIZE>
template <typename PROCESSOR>
requires Processor<PROCESSOR, typename ENCODER::MessageType> void
         Connection<ENCODER, BUF_SIZE>::process(PROCESSOR& processor)
{
    read();
    while (std::optional<MessageType> message = ENCODER::consume_message(
               m_incoming)) {
        // do action on the read message
        if (std::optional<MessageType> response = processor.process(
                std::move(*message))) {
            ENCODER::write(std::move(*response), m_outgoing);
            m_signals.write = true;
            m_signals.read  = false;
        }
    }

    if (!m_outgoing.empty()) {
        // try writing right away if its full then EAGAIN will be handled
        write();
    }
}

}  // namespace net
#endif
