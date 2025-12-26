#ifndef INCLUDED_REDIS_SERVER_H
#define INCLUDED_REDIS_SERVER_H

#include <net_server.h>
#include <net_tagged_encoder.h>

namespace redis {

using Messages   = net::Messages<std::string, std::vector<std::string> >;
using KeyEncoder = net::TaggedEncoder<net::Codec, Messages>;

class RedisProcessor {
  public:
    std::optional<Messages::MessageVariant>
    process(Messages::MessageVariant&& request);
};

using RedisServer = net::Server<net::Connection<KeyEncoder>, RedisProcessor>;

}  // namespace redis

#endif
