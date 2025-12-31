#ifndef INCLUDE_NET_CONNECTION_H
#define INCLUDE_NET_CONNECTION_H

#include "net_codec.h"
#include "net_file_descriptor.h"

#include <csignal>
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
};

class Connection {
    FileDescriptor m_fd;
    bool           m_closed = false;

  protected:
    template <ReadBuffer BUFFER>
    void read(BUFFER& incoming);

  public:
    Connection(FileDescriptor&& fd)
    : m_fd{std::move(fd)}
    {
    }
    const FileDescriptor& fd() const { return m_fd; }

    void close() { m_closed = true; }
    bool is_closed() const { return m_closed; }
};

template <ReadBuffer BUFFER>
void Connection::read(BUFFER& incoming)
{
    std::cout << "[SERVER][CONNECTION][READ] receiving server message "
                 "on connection fd: "
              << m_fd << std::endl;
    ssize_t rc = incoming.read_from(m_fd);
    if (rc < 0) {
        close();
        std::cout
            << "[SERVER][CONNECTION][READ] connection closed by peer fd: "
            << m_fd << std::endl;
        return;
    }
}

}  // namespace net
#endif
