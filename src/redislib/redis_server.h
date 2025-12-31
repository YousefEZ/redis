#ifndef INCLUDED_REDIS_SERVER_H
#define INCLUDED_REDIS_SERVER_H

#include <net_polled_connection.h>
#include <net_server.h>
#include <net_tagged_encoder.h>

namespace redis {

using MessageTypes = net::Messages<std::string, uint32_t>;
using KeyEncoder   = net::TaggedEncoder<net::Codec, MessageTypes>;

class RedisProcessor {
  public:
    std::optional<MessageTypes::MessageVariant>
    process(MessageTypes::MessageVariant request);
};

using RedisServer =
    net::Server<net::PolledConnection<KeyEncoder>, RedisProcessor>;

}  // namespace redis

#endif
