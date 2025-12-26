#ifndef INCLUDED_NET_SERVER_H
#define INCLUDED_NET_SERVER_H

#include "net_connection.h"

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

template <typename CONNECTION>
void prepare_poll_events(std::vector<pollfd>&           polls,
                         const std::vector<CONNECTION>& connections)
{
    polls.reserve(connections.size());
    std::cout << "[SERVER][POLL][PREPARE] preparing poll events for "
              << connections.size() << " connections" << std::endl;
    polls.emplace_back(connections[0].fd(), POLLIN, 0);

    std::ranges::transform(connections | std::views::drop(1),
                           std::back_inserter(polls),
                           [](const CONNECTION& conn) -> pollfd {
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

}  // namespace detail

template <typename CONNECTION, typename PROCESSOR>
class Server {
    const sockaddr_in       m_address;
    std::vector<CONNECTION> m_connections;
    PROCESSOR               m_processor;

    [[nodiscard]] const CONNECTION& socket_connection() const;

    void execute_connection_events(const pollfd& poll, CONNECTION& connection);
    void remove_closed_connections();
    void accept_connection(const pollfd& socket_poll);
    void check_connections(const std::vector<pollfd>& poll_event);

  public:
    void run();

    Server(sockaddr_in&& address);
};

namespace {
}  // namespace

template <typename CONNECTION, typename PROCESSOR>
Server<CONNECTION, PROCESSOR>::Server(sockaddr_in&& address)
: m_address{address}
{
    m_connections.emplace_back(detail::setup_listener(m_address));
    m_connections.back().fd().as_non_blocking();
}

template <typename CONNECTION, typename PROCESSOR>
const CONNECTION& Server<CONNECTION, PROCESSOR>::socket_connection() const
{
    return m_connections[0];
}

template <typename CONNECTION, typename PROCESSOR>
void Server<CONNECTION, PROCESSOR>::execute_connection_events(
    const pollfd& poll,
    CONNECTION&   connection)
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

template <typename CONNECTION, typename PROCESSOR>
void Server<CONNECTION, PROCESSOR>::check_connections(
    const std::vector<pollfd>& polls)
{
    std::ranges::for_each(std::views::iota(size_t{1}, polls.size()),
                          [&polls, this](const int idx) {
                              execute_connection_events(polls[idx],
                                                        m_connections[idx]);
                          });
}

template <typename CONNECTION, typename PROCESSOR>
void Server<CONNECTION, PROCESSOR>::remove_closed_connections()
{
    const auto [first,
                last] = std::ranges::remove_if(m_connections,
                                               [](const auto& conn) -> bool {
                                                   return conn.want_close();
                                               });
    m_connections.erase(first, last);
}

template <typename CONNECTION, typename PROCESSOR>
void Server<CONNECTION, PROCESSOR>::accept_connection(
    const pollfd& socket_poll)
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

template <typename CONNECTION, typename PROCESSOR>
void Server<CONNECTION, PROCESSOR>::run()
{
    std::vector<pollfd> polls;
    while (true) {
        prepare_poll_events(polls, m_connections);
        detail::run_poll(polls);

        check_connections(polls);
        remove_closed_connections();
        accept_connection(polls[0]);

        polls.clear();
    }
}

}  // namespace net

#endif
