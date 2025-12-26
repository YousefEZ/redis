#include "net_server.h"
#include "net_utils.h"

#include <arpa/inet.h>
#include <cassert>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <netinet/ip.h>
#include <poll.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

#define SOCKET_ENABLE 1

namespace net {

namespace detail {

FileDescriptor setup_listener(const sockaddr_in& address)
{
    FileDescriptor fd = socket(AF_INET, SOCK_STREAM, 0);

    std::cout << "[SERVER][SETUP][LISTENER] Created socket with fd: " << fd
              << std::endl;

    int opt = SOCKET_ENABLE;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    std::cout << "[SERVER][SETUP][LISTENER] Binding to "
              << inet_ntoa(address.sin_addr) << ":" << ntohs(address.sin_port)
              << std::endl;

    int rc = bind(fd, (const sockaddr*)&address, sizeof(address));

    utils::die_on(
        rc,
        "[SERVER][SETUP][LISTENER] unable to bind addr, shutting down");

    rc = listen(fd, SOMAXCONN);

    utils::die_on(rc,
                  "[SERVER][SETUP][LISTENER] unable to start listening on "
                  "addr, shutting down");

    return fd;
}

void run_poll(std::vector<pollfd>& polls)
{
    int rc = poll(polls.data(), polls.size(), -1);
    if (rc < 0 && errno == EINTR) {
        return;  // not an error
    }
    utils::die_on(rc < 0, "unable to poll");
}

}  // namespace detail
}  // namespace net
