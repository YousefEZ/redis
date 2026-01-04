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

    inline ResponseEncoder::MessageType get(std::string key)
    {
        m_conn.send(GetRequest{key});
        return m_conn.get_response();
    }

    inline ResponseEncoder::MessageType set(std::string key, std::string value)
    {
        m_conn.send(SetRequest<std::string>{key, value});
        return m_conn.get_response();
    }

    inline ResponseEncoder::MessageType
    request(const RequestEncoder::MessageType& req);
};

}  // namespace redis

#endif
