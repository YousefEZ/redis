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

namespace {

void set_fd_as_nonblocking(int fd) {

  fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK);
  std::cout << "[SERVER][SET][NONBLOCK] Set fd: " << fd << " as non-blocking"
            << std::endl;
}

FileDescriptor setup_listener(const sockaddr_in &address) {
  int fd = socket(AF_INET, SOCK_STREAM, 0);

  std::cout << "[SERVER][SETUP][LISTENER] Created socket with fd: " << fd
            << std::endl;

  int val = 1;
  setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));

  std::cout << "[SERVER][SETUP][LISTENER] Binding to "
            << inet_ntoa(address.sin_addr) << ":" << ntohs(address.sin_port)
            << std::endl;

  int rc = bind(fd, (const sockaddr *)&address, sizeof(address));

  utils::die_on(rc,
                "[SERVER][SETUP][LISTENER] unable to bind addr, shutting down");

  rc = listen(fd, SOMAXCONN);

  utils::die_on(rc, "[SERVER][SETUP][LISTENER] unable to start listening on "
                    "addr, shutting down");

  return fd;
}

void prepare_poll_events(std::vector<pollfd> &polls,
                         const std::vector<detail::Connection> &connections) {
  polls.reserve(connections.size());
  std::cout << "[SERVER][POLL][PREPARE] preparing poll events for "
            << connections.size() << " connections" << std::endl;
  polls.emplace_back(connections[0].m_fd, POLLIN, 0);

  std::ranges::transform(connections | std::views::drop(1),
                         std::back_inserter(polls),
                         [](const detail::Connection &conn) -> pollfd {
                           short events = POLLERR;
                           events |= conn.m_want_read ? POLLIN : 0;
                           events |= conn.m_want_write ? POLLOUT : 0;

                           return {
                               .fd = conn.m_fd,
                               .events = events,
                               .revents = 0,
                           };
                         });
}

void run_poll(std::vector<pollfd> &polls) {
  int rc = poll(polls.data(), polls.size(), -1);
  if (rc < 0 && errno == EINTR) {
    return; // not an error
  }
  utils::die_on(rc < 0, "unable to poll");
}

void execute_connection_events(const pollfd &poll_event,
                               detail::Connection &conn) {

  if ((poll_event.revents & POLLIN) && conn.m_want_read)
    conn.read();

  if ((poll_event.revents & POLLOUT) && conn.m_want_write)
    conn.write();

  if ((poll_event.revents & POLLERR) || conn.m_want_close) {
    close(conn.m_fd);
    conn.m_want_close = true;
    std::cout << "[SERVER][EXECUTE][CLOSE] closed connection fd: " << conn.m_fd
              << std::endl;
  }
}

} // namespace

Server::Server(sockaddr_in &&address) : m_address{address} {
  int socket_fd = setup_listener(m_address);
  m_connections.emplace_back(socket_fd);
  set_fd_as_nonblocking(socket_fd);
}

const detail::Connection &Server::socket_connection() const {
  return m_connections[0];
}

void Server::check_connections(const std::vector<pollfd> &polls) {
  std::ranges::for_each(
      std::views::iota(size_t{1}, polls.size()), [&polls, this](const int idx) {
        execute_connection_events(polls[idx], m_connections[idx]);
      });
}

void Server::remove_closed_connections() {
  const auto [first, last] =
      std::ranges::remove_if(m_connections, [](const auto &conn) -> bool {
        return conn.m_want_close;
      });
  m_connections.erase(first, last);
}

void Server::accept_connection(const pollfd &socket_poll) {
  if (socket_poll.revents & !POLLIN) {
    return;
  }

  struct sockaddr_in client_addr = {};

  socklen_t addrlen = sizeof(client_addr);
  int connfd =
      accept(socket_connection().m_fd, (sockaddr *)&client_addr, &addrlen);
  if (connfd < 0) {
    return;
  }
  std::cout << "[SERVER][CONNECTION][ACCEPT] Accepted new connection with fd: "
            << connfd << std::endl;
  set_fd_as_nonblocking(connfd);
  m_connections.emplace_back(connfd);
  m_connections.back().m_want_read = true;
}

namespace detail {

static const char ACK[] = "ACK";

void Connection::read() {
  std::cout << "[SERVER][CONNECTION][READ] receiving server message "
               "on connection fd: "
            << m_fd << std::endl;
  ssize_t rc = receive_message(m_fd, m_incoming);
  if (rc <= 0) {
    m_want_close = true;
    std::cout << "[SERVER][CONNECTION][READ] connection closed by peer fd: "
              << m_fd << std::endl;
    return;
  }
  // read the vector and determine whether the full message is there
  if (std::optional<std::string> message = consume_message(m_incoming)) {
    // do action on the read message
    std::cout << "[SERVER][CONNECTION][READ] message=" << message.value()
              << std::endl;
    m_want_write = true;
    m_want_read = false;
    m_outgoing.insert(m_outgoing.end(), ACK, ACK + sizeof(ACK));
  } else {
    std::cout << "[SERVER][CONNECTION][READ] incomplete message, waiting for "
                 "more data"
              << std::endl;
  }
}

void Connection::write() {
  if (m_outgoing.size() == 0) {
    m_want_read = true;
    m_want_write = false;
    return;
  }
  send_message(m_fd, m_outgoing.data(), m_outgoing.size());
  m_outgoing.clear();
  std::cout << "[SERVER][CONNECTION][WRITE] sent server message on "
               "connection fd: "
            << m_fd << std::endl;
}

} // namespace detail

void Server::run() {
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
