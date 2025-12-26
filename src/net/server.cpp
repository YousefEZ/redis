#include "server.h"
#include "connection.h"
#include "utils.h"

#include <algorithm>
#include <arpa/inet.h>
#include <cassert>
#include <cerrno>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <netinet/ip.h>
#include <poll.h>
#include <ranges>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

#define SOCKET_ENABLE 1

namespace {

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

void prepare_poll_events(std::vector<pollfd>&                 polls,
                         const std::vector<StringConnection>& connections)
{
    polls.reserve(connections.size());
    std::cout << "[SERVER][POLL][PREPARE] preparing poll events for "
              << connections.size() << " connections" << std::endl;
    polls.emplace_back(connections[0].fd(), POLLIN, 0);

    std::ranges::transform(connections | std::views::drop(1),
                           std::back_inserter(polls),
                           [](const StringConnection& conn) -> pollfd {
                               short events = POLLERR;
                               events |= conn.want_read() ? POLLIN : 0;
                               events |= conn.want_write() ? POLLOUT : 0;

                               return {
                                   .fd      = conn.fd(),
                                   .events  = events,
                                   .revents = 0,
                               };
                           });
}

void run_poll(std::vector<pollfd>& polls)
{
    int rc = poll(polls.data(), polls.size(), -1);
    if (rc < 0 && errno == EINTR) {
        return;  // not an error
    }
    utils::die_on(rc < 0, "unable to poll");
}

}  // namespace

std::optional<std::string> ServerProcessor::process(const std::string& message)
{
    std::cout << "[SERVER][PROCESSOR][PROCESS] processing message: " << message
              << std::endl;
    return std::optional<std::string>("PONG");
}

Server::Server(sockaddr_in&& address)
: m_address{address}
{
    m_connections.emplace_back(setup_listener(m_address));
    m_connections.back().fd().as_non_blocking();
}

const StringConnection& Server::socket_connection() const
{
    return m_connections[0];
}

void Server::execute_connection_events(const pollfd&     poll,
                                       StringConnection& connection)
{
    if (poll.revents & POLLERR) {
        std::cout << "[SERVER][CONNECTION][ERROR] error on connection fd: "
                  << connection.fd() << std::endl;
        connection.close();
        return;
    }
    if (poll.revents & POLLIN) {
        connection.process(m_processor);
    }
    if (poll.revents & POLLOUT) {
        connection.write();
    }
}

void Server::check_connections(const std::vector<pollfd>& polls)
{
    std::ranges::for_each(std::views::iota(size_t{1}, polls.size()),
                          [&polls, this](const int idx) {
                              execute_connection_events(polls[idx],
                                                        m_connections[idx]);
                          });
}

void Server::remove_closed_connections()
{
    const auto [first,
                last] = std::ranges::remove_if(m_connections,
                                               [](const auto& conn) -> bool {
                                                   return conn.want_close();
                                               });
    m_connections.erase(first, last);
}

void Server::accept_connection(const pollfd& socket_poll)
{
    if (socket_poll.revents & !POLLIN) {
        return;
    }

    struct sockaddr_in client_addr = {};

    socklen_t      addrlen = sizeof(client_addr);
    FileDescriptor connfd  = accept(socket_connection().fd(),
                                   (sockaddr*)&client_addr,
                                   &addrlen);
    if (connfd < 0) {
        return;
    }
    connfd.as_non_blocking();
    std::cout
        << "[SERVER][CONNECTION][ACCEPT] Accepted new connection with fd: "
        << connfd << std::endl;
    m_connections.emplace_back(std::move(connfd), Signals{.read = true});
}

void Server::run()
{
    std::vector<pollfd> polls;
    while (true) {
        prepare_poll_events(polls, m_connections);
        run_poll(polls);

        check_connections(polls);
        remove_closed_connections();
        accept_connection(polls[0]);

        polls.clear();
    }
}
