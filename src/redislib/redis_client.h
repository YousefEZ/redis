#ifndef INCLUDED_REDISLIB_CLIENT_H
#define INCLUDED_REDISLIB_CLIENT_H

#include "redis_schema.h"

#include <net_blocking_connection.h>
#include <net_codec.h>
#include <net_single_type_encoder.h>
#include <net_tagged_encoder.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

namespace redis {

using SyncConnection =
    net::BlockingConnection<RequestEncoder, ResponseEncoder>;

class SyncClient {
    SyncConnection m_conn;

  public:
    SyncClient(SyncConnection&& conn)
    : m_conn(std::move(conn))
    {
    }

    inline ResponseEncoder::MessageType
    request(const RequestEncoder::MessageType& req);
};

ResponseEncoder::MessageType
SyncClient::request(const RequestEncoder::MessageType& req)
{
    m_conn.send(req);
    return m_conn.get_response();
}

}  // namespace redis

#endif
