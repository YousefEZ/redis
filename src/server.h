#ifndef REDIS_SERVER_H
#define REDIS_SERVER_H

#include <arpa/inet.h>
#include <netinet/in.h>
#include <poll.h>
#include <sys/socket.h>
#include <vector>

typedef int FileDescriptor;

namespace detail {
struct Connection {
  int m_fd;

  bool m_want_read = false;
  bool m_want_write = false;
  bool m_want_close = false;

  std::vector<char> m_incoming;
  std::vector<char> m_outgoing;

  void read();
  void write();
};

} // namespace detail

class Server {
  const sockaddr_in m_address;
  std::vector<detail::Connection> m_connections;

  [[nodiscard]] const detail::Connection &socket_connection() const;

  void remove_closed_connections();
  void accept_connection(const pollfd &socket_poll);
  void check_connections(const std::vector<pollfd> &poll_event);

public:
  void run();

  Server(sockaddr_in &&address);
};

#endif
