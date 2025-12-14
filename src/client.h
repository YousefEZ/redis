#ifndef REDIS_CLIENT_H
#define REDIS_CLIENT_H

#include "connection.h"
#include "message_parsing.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

class Client {

  Connection<StringEncoder> m_conn;

public:
  Client(Connection<StringEncoder> &&conn) : m_conn(std::move(conn)) {};

  void run();
};

#endif
