#ifndef REDIS_CLIENT_H
#define REDIS_CLIENT_H

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

class Client {

  sockaddr_in addr;

public:
  Client(sockaddr_in address);
  void run();
};

#endif
