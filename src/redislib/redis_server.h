#ifndef INCLUDED_REDIS_SERVER_H
#define INCLUDED_REDIS_SERVER_H

#include <net_codec.h>
#include <net_polled_connection.h>
#include <net_server.h>
#include <net_single_type_encoder.h>
#include <net_tagged_encoder.h>

#include <optional>

namespace redis {

using MessageTypes = net::Messages<std::string, uint32_t>;
using KeyEncoder   = net::TaggedEncoder<net::Codec, MessageTypes>;

class RedisProcessor {
  public:
    std::optional<KeyEncoder::MessageType>
    process(KeyEncoder::MessageType request);
};

using RedisServer = net::Server<KeyEncoder, RedisProcessor>;

}  // namespace redis

#endif
