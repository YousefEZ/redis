#ifndef INCLUDED_REDISLIB_CLIENT_H
#define INCLUDED_REDISLIB_CLIENT_H

#include "net_blocking_connection.h"
#include "net_codec.h"
#include "net_single_type_encoder.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <string>

namespace redis {

typedef net::SingleTypeEncoder<net::Codec, std::string> StringEncoder;
using SyncConnection = net::BlockingConnection<StringEncoder>;

class SyncClient {
    SyncConnection m_conn;

  public:
    SyncClient(SyncConnection&& conn)
    : m_conn(std::move(conn))
    {
    }

    inline StringEncoder::MessageType
    request(const StringEncoder::MessageType& req);
};

StringEncoder::MessageType
SyncClient::request(const StringEncoder::MessageType& req)
{
    m_conn.send(req);
    return m_conn.get_response();
}

}  // namespace redis

#endif
