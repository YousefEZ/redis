#ifndef REDIS_SERVER_H
#define REDIS_SERVER_H

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

class Server {
  const sockaddr_in m_address;

  void communicate(int connfd);

public:
  void run();

  Server(sockaddr_in &&address);
};

#endif
