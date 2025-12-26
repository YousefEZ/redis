#ifndef INCLUDED_REDISLIB_CLIENT_H
#define INCLUDED_REDISLIB_CLIENT_H

#include "net_codec.h"
#include <net_connection.h>
#include <net_single_type_encoder.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <string>

namespace redis {

typedef net::SingleTypeEncoder<net::Codec, std::string> StringEncoder;

class AsyncClient {};

class Client {
    net::Connection<StringEncoder> m_conn;

  public:
    Client(net::Connection<StringEncoder>&& conn)
    : m_conn(std::move(conn)) {};

    void run();
};
}  // namespace redis

#endif
