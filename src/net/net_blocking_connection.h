#ifndef INCLUDE_NET_BLOCKING_CONNECTION_H
#define INCLUDE_NET_BLOCKING_CONNECTION_H

#include "net_buffer.h"
#include "net_connection.h"
#include "net_encoder.h"
#include "net_file_descriptor.h"
#include "net_utils.h"

#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>

namespace net {

class SocketWriteBuffer {
    const FileDescriptor& m_fd;
    ssize_t               m_capacity;

    static ssize_t get_capacity(const FileDescriptor& fd)
    {
        int       rc;
        socklen_t len = sizeof(rc);
        ssize_t   capacity;
        rc = getsockopt(fd, SOL_SOCKET, SO_SNDBUF, &capacity, &len);
        utils::die_on(rc != 0, "unable to get socket send buffer size");
        return capacity;
    }

  public:
    SocketWriteBuffer(const FileDescriptor& fd)
    : m_fd{fd}
    , m_capacity{get_capacity(fd)}
    {
    }

    ssize_t size() const
    {
        int available;
        ioctl(m_fd, FIONREAD, &available);
        return available;
    }

    ssize_t capacity() const { return m_capacity; }

    void append(const char* buf, ssize_t n) const
    {
        net::utils::write_full(m_fd, (char*)buf, n);
    }
};

template <typename ENCODER,
          typename DECODER,
          ssize_t BUF_SIZE = MAX_BUFFER_SIZE>
requires
    net::Serde<ENCODER, SocketWriteBuffer, Buffer> class BlockingConnection
: public Connection {
    using RequestType  = typename ENCODER::MessageType;
    using ResponseType = typename DECODER::MessageType;

    Buffer            m_incoming{BUF_SIZE};
    SocketWriteBuffer m_outgoing;

  public:
    BlockingConnection(FileDescriptor&& fd)
    : Connection{std::move(fd)}
    , m_outgoing{Connection::fd()}
    {
    }

    BlockingConnection(BlockingConnection&& other)
    : Connection{std::move(other)}
    , m_outgoing{Connection::fd()}
    {
    }

    template <typename MESSAGE>
    void send(MESSAGE&& message);

    ResponseType get_response();
};

template <typename ENCODER, typename DECODER, ssize_t BUF_SIZE>
requires
    net::Serde<ENCODER, SocketWriteBuffer, Buffer> template <typename MESSAGE>
    void
    BlockingConnection<ENCODER, DECODER, BUF_SIZE>::send(MESSAGE&& message)
{
    ENCODER::write(std::forward<MESSAGE>(message), m_outgoing);
}

template <typename ENCODER, typename DECODER, ssize_t BUF_SIZE>
requires net::Serde<ENCODER, SocketWriteBuffer, Buffer> DECODER::MessageType
BlockingConnection<ENCODER, DECODER, BUF_SIZE>::get_response()
{
    while (true) {
        read(m_incoming);
        if (std::optional<ResponseType> message = DECODER::consume_message(
                m_incoming)) {
            return *message;
        }
    }
}
}  // namespace net

#endif
