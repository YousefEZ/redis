#ifndef INCLUDED_REDIS_SCHEMA_H
#define INCLUDED_REDIS_SCHEMA_H

#include <net_codec.h>
#include <net_tagged_encoder.h>
#include <string>

struct GetRequest {
    std::string key;
};

struct GetResponse {};

struct SetRequest {};

struct SetResponse {};

using MessageTypes = net::Messages<std::string, uint32_t>;

using Requests =
    net::Messages<std::string>;  // net::Messages<GetRequest, SetRequest>;
using Responses = net::Messages<std::string>;  // net::Messages<GetResponse,
                                               // SetResponse, std::string>;

using RequestEncoder  = net::TaggedEncoder<net::Codec, Requests>;
using ResponseEncoder = net::TaggedEncoder<net::Codec, Responses>;

#endif
