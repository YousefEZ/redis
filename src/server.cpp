#include "server.h"
#include "message_parsing.h"
#include "utils.h"

#include <algorithm>
#include <arpa/inet.h>
#include <cassert>
#include <cerrno>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <netinet/ip.h>
#include <poll.h>
#include <ranges>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

namespace {

FileDescriptor setup_listener(const sockaddr_in &address) {
  int fd = socket(AF_INET, SOCK_STREAM, 0);

  std::cout << "[SERVER][RUN] Created socket with fd: " << fd << std::endl;

  int val = 1;
  setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));

  std::cout << "[SERVER][RUN] Binding to " << inet_ntoa(address.sin_addr) << ":"
            << ntohs(address.sin_port) << std::endl;

  int rc = bind(fd, (const sockaddr *)&address, sizeof(address));

  utils::die_on(rc, "[SERVER][RUN] unable to bind addr, shutting down");

  rc = listen(fd, SOMAXCONN);

  utils::die_on(
      rc, "[SERVER][RUN] unable to start listening on addr, shutting down");

  return fd;
}

} // namespace

Server::Server(sockaddr_in &&address) : m_address{address} {
  m_connections.emplace_back(setup_listener(m_address));
}

void Server::communicate(int fd) {
  char rbuf[MAX_MESSAGE_LENGTH] = {};
  receive_message(fd, rbuf);

  std::cout << "[SERVER][COMMUNICATE] received msg: " << rbuf << std::endl;

  char response[] = "ACK";
  send_message(fd, response, sizeof(response));
}

const detail::Connection &Server::socket_connection() const {
  return m_connections[0];
}

void Server::accept_connection() {
  struct sockaddr_in client_addr = {};

  socklen_t addrlen = sizeof(client_addr);
  int connfd =
      accept(socket_connection().m_fd, (sockaddr *)&client_addr, &addrlen);
  if (connfd < 0) {
    return;
  }

  m_connections.emplace_back(connfd);
}

void Server::check_connection(const pollfd &poll_event, const int idx) {
  auto conn = m_connections[idx];
  if (poll_event.revents & POLLIN) {
    receive_message(conn.m_fd, conn.m_incoming.data());
  }

  if (poll_event.revents & POLLOUT) {
    send_message(conn.m_fd, conn.m_outgoing.data(), conn.m_outgoing.size());
  }
}

void Server::check_socket() {}

void Server::run() {
  std::vector<pollfd> polls;
  while (true) {
    polls.reserve(m_connections.size()); // we know the size before hand

    polls.emplace_back(socket_connection().m_fd, POLLIN, 0);

    std::ranges::transform(
        m_connections | std::views::drop(1), std::back_inserter(polls),
        [](const detail::Connection &conn) -> pollfd {
          return {
              .fd = conn.m_fd,
              .events = static_cast<short>((POLLIN * conn.m_want_read) |
                                           (POLLOUT * conn.m_want_write)),
              .revents = 0,
          };
        });

    int rc = poll(polls.data(), polls.size(), -1);

    if (rc < 0 && errno == EINTR) {
      continue; // not an error
    }

    utils::die_on(rc, "unable to poll");

    std::ranges::for_each(
        std::views::iota(size_t{1}, polls.size()),
        [&polls, this](const int idx) { check_connection(polls[idx], idx); });

    const auto [first, last] =
        std::ranges::remove_if(m_connections, [](const auto &conn) -> bool {
          return !conn.m_want_close;
        });
    m_connections.erase(first, last);

    accept_connection();

    polls.clear();
  }
}
