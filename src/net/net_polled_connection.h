#ifndef INCLUDE_NET_POLLED_CONNECTION_H
#define INCLUDE_NET_POLLED_CONNECTION_H

#include "net_buffer.h"
#include "net_connection.h"
#include "net_encoder.h"

#include <sys/poll.h>

namespace net {

template <typename ENCODER, ssize_t BUF_SIZE = MAX_BUFFER_SIZE>
requires net::Serde<ENCODER, Buffer, Buffer> class PolledConnection
: public Connection {
    typedef ENCODER::MessageType MessageType;

    Signals m_signals;

    Buffer m_incoming{BUF_SIZE};
    Buffer m_outgoing{BUF_SIZE};

  public:
    PolledConnection(FileDescriptor&& fd, Signals signals = {})
    : Connection{std::move(fd)}
    , m_signals(signals)
    {
    }

    pollfd get_pollfd() const
    {
        std::cout << "[SERVER][POLL][GET] POLLFD for connection fd: " << fd()
                  << ", with signals read: " << m_signals.read
                  << ", write: " << m_signals.write << std::endl;

        return pollfd{fd(),
                      static_cast<short>((m_signals.read ? POLLIN : 0) |
                                         (m_signals.write ? POLLOUT : 0) |
                                         POLLERR),
                      0};
    }

    void write();

    template <typename PROCESSOR>
    requires Processor<PROCESSOR, MessageType> void
             process(PROCESSOR& processor);
};

template <typename ENCODER, ssize_t BUF_SIZE>
requires net::Serde<ENCODER, Buffer, Buffer> void
         PolledConnection<ENCODER, BUF_SIZE>::write()
{
    if (m_outgoing.size() == 0) {
        m_signals.read  = true;
        m_signals.write = false;
        return;
    }
    if (m_outgoing.write_to(fd(), m_outgoing.size()) < 0) {
        if (errno != EAGAIN) {
            close();
            std::cout
                << "[SERVER][CONNECTION][WRITE] connection closed by peer fd: "
                << fd() << std::endl;
            return;
        }
        std::cout << "[SERVER][CONNECTION][WRITE] EAGAIN on connection fd: "
                  << fd() << std::endl;
    }
    else {
        m_outgoing.clear();
        std::cout << "[SERVER][CONNECTION][WRITE] sent server message on "
                     "connection fd: "
                  << fd() << std::endl;
    }
}

template <typename ENCODER, ssize_t BUF_SIZE>
requires net::Serde<ENCODER, Buffer, Buffer> template <typename PROCESSOR>
requires Processor<PROCESSOR, typename ENCODER::MessageType> void
         PolledConnection<ENCODER, BUF_SIZE>::process(PROCESSOR& processor)
{
    read(m_incoming);
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
