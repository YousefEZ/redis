#ifndef INCLUDED_NET_SERVER_H
#define INCLUDED_NET_SERVER_H

#include "net_file_descriptor.h"
#include "net_polled_connection.h"

#include <algorithm>
#include <ranges>
#include <vector>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <poll.h>
#include <sys/socket.h>

namespace net {

namespace detail {

FileDescriptor setup_listener(const sockaddr_in& address);
void           run_poll(std::vector<pollfd>& polls);

template <typename ENCODER>
void prepare_poll_events(
    const FileDescriptor&                               listen_fd,
    std::vector<pollfd>&                                polls,
    const std::vector<net::PolledConnection<ENCODER> >& connections)
{
    polls.reserve(connections.size());
    std::cout << "[SERVER][POLL][PREPARE] preparing poll events for "
              << connections.size() << " connections" << std::endl;
    polls.emplace_back(listen_fd, POLLIN, 0);

    std::ranges::transform(connections | std::views::drop(1),
                           std::back_inserter(polls),
                           &net::PolledConnection<ENCODER>::get_pollfd);
}

}  // namespace detail

template <typename ENCODER, typename PROCESSOR>
class Server {
    FileDescriptor                               m_listen_fd;
    std::vector<net::PolledConnection<ENCODER> > m_connections;
    PROCESSOR&                                   m_processor;

    void execute_connection_events(const pollfd&                   poll,
                                   net::PolledConnection<ENCODER>& connection);
    void remove_closed_connections();
    void accept_connection(const pollfd& socket_poll);
    void check_connections(const std::vector<pollfd>& poll_event);

  public:
    void run();

    Server(sockaddr_in&& address, PROCESSOR& processor);
};

template <typename ENCODER, typename PROCESSOR>
Server<ENCODER, PROCESSOR>::Server(sockaddr_in&& address, PROCESSOR& processor)
: m_listen_fd{detail::setup_listener(std::move(address))}
, m_processor{processor}
{
    m_connections.back().fd().as_non_blocking();
}

template <typename ENCODER, typename PROCESSOR>
void Server<ENCODER, PROCESSOR>::execute_connection_events(
    const pollfd&                   poll,
    net::PolledConnection<ENCODER>& connection)
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

template <typename ENCODER, typename PROCESSOR>
void Server<ENCODER, PROCESSOR>::check_connections(
    const std::vector<pollfd>& polls)
{
    std::ranges::for_each(std::views::iota(size_t{1}, polls.size()),
                          [&polls, this](const int idx) {
                              execute_connection_events(polls[idx],
                                                        m_connections[idx]);
                          });
}

template <typename ENCODER, typename PROCESSOR>
void Server<ENCODER, PROCESSOR>::remove_closed_connections()
{
    const auto [first,
                last] = std::ranges::remove_if(m_connections,
                                               [](const auto& conn) -> bool {
                                                   return conn.is_closed();
                                               });
    m_connections.erase(first, last);
}

template <typename ENCODER, typename PROCESSOR>
void Server<ENCODER, PROCESSOR>::accept_connection(const pollfd& socket_poll)
{
    if (socket_poll.revents & !POLLIN) {
        return;
    }

    struct sockaddr_in client_addr = {};

    socklen_t      addrlen = sizeof(client_addr);
    FileDescriptor connfd  = accept(m_listen_fd,
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

template <typename ENCODER, typename PROCESSOR>
void Server<ENCODER, PROCESSOR>::run()
{
    std::vector<pollfd> polls;
    while (true) {
        detail::prepare_poll_events(m_listen_fd, polls, m_connections);
        detail::run_poll(polls);

        check_connections(polls);
        remove_closed_connections();
        accept_connection(polls[0]);

        polls.clear();
    }
}

}  // namespace net

#endif
