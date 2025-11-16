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
};

} // namespace detail

class Server {
  const sockaddr_in m_address;
  std::vector<detail::Connection> m_connections;

  void communicate(int connfd);
  void accept_connection();
  [[nodiscard]] const detail::Connection &socket_connection() const;

  void check_connection(const pollfd &poll_event, const int conn_idx);
  void check_socket();

public:
  void run();

  Server(sockaddr_in &&address);
};

#endif
