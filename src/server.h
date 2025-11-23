#ifndef REDIS_SERVER_H
#define REDIS_SERVER_H

#include "connection.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <poll.h>
#include <sys/socket.h>
#include <vector>

class Server {
  const sockaddr_in m_address;
  std::vector<Connection> m_connections;

  [[nodiscard]] const Connection &socket_connection() const;

  void remove_closed_connections();
  void accept_connection(const pollfd &socket_poll);
  void check_connections(const std::vector<pollfd> &poll_event);

public:
  void run();

  Server(sockaddr_in &&address);
};

#endif
