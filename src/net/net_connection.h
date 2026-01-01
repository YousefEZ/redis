#ifndef INCLUDE_NET_CONNECTION_H
#define INCLUDE_NET_CONNECTION_H

#include "net_file_descriptor.h"

#include <csignal>
#include <iostream>
#include <optional>
#include <sys/poll.h>
#include <sys/types.h>
#include <utility>

#define MAX_BUFFER_SIZE 4096
namespace net {

template <typename PROCESSOR, typename INCOMING, typename OUTGOING>
concept Processor = requires(PROCESSOR & processor, INCOMING message)
{
    {processor.process(message)}->std::same_as<std::optional<OUTGOING> >;
};

struct Signals {
    bool read;
    bool write;
};

class Connection {
    FileDescriptor m_fd;
    bool           m_closed = false;

  protected:
    template <typename BUFFER>
    void read(BUFFER& incoming);

  public:
    Connection(FileDescriptor&& fd)
    : m_fd{std::move(fd)}
    {
        std::cout << "[SERVER][CONNECTION] new connection fd: " << m_fd
                  << std::endl;
    }

    Connection(Connection&& other)
    : m_fd{std::move(other.m_fd)}
    {
    }

    Connection& operator=(Connection&& other)
    {
        if (this != &other) {
            m_fd = std::move(other.m_fd);
        }
        return *this;
    }

    Connection(const Connection&)            = delete;
    Connection& operator=(const Connection&) = delete;
    ~Connection()                            = default;

    const FileDescriptor& fd() const { return m_fd; }

    void close() { m_closed = true; }
    bool is_closed() const { return m_closed; }
};

template <typename BUFFER>
void Connection::read(BUFFER& incoming)
{
    if (is_closed()) {
        std::cout
            << "[SERVER][CONNECTION][READ] connection already closed fd: "
            << fd() << std::endl;
        return;
    }
    std::cout << "[SERVER][CONNECTION][READ] receiving server message "
                 "on connection fd: "
              << fd() << std::endl;

    ssize_t rc = incoming.read_from(fd());
    if (rc <= 0) {
        close();
        std::cout
            << "[SERVER][CONNECTION][READ] connection closed by peer fd: "
            << fd() << std::endl;
        return;
    }
}

}  // namespace net
#endif
