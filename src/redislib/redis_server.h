#ifndef INCLUDED_REDIS_SERVER_H
#define INCLUDED_REDIS_SERVER_H

#include "redis_schema.h"

#include <net_codec.h>
#include <net_polled_connection.h>
#include <net_server.h>
#include <net_single_type_encoder.h>
#include <net_tagged_encoder.h>

#include <optional>

namespace redis {

class RedisProcessor {
  public:
    std::optional<ResponseEncoder::MessageType>
    process(RequestEncoder::MessageType request);
};

using RedisServer =
    net::Server<RequestEncoder, ResponseEncoder, RedisProcessor>;

}  // namespace redis

#endif
