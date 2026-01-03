#ifndef INCLUDED_REDIS_SCHEMA_H
#define INCLUDED_REDIS_SCHEMA_H

#include "redis_meta.h"

#include <net_codec.h>
#include <net_tagged_encoder.h>

#include <string>

namespace redis {

struct GetRequest {
    std::string key;
};

template <typename T>
struct GetResponse {
    T value;
};

template <typename T>
struct SetRequest {
    std::string key;
    T           value;
};

struct SetResponse {
    bool success;
};

using TypeValues = meta::TypeList<std::string, int, bool>;

using MyRequests =
    meta::Append<TypeValues::Apply<SetRequest>, GetRequest>::Value;
using MyResponses =
    meta::Append<TypeValues::Apply<GetResponse>, SetResponse>::Value;

using Requests  = meta::As<MyRequests>::Messages;
using Responses = meta::As<MyResponses>::Messages;

using RequestEncoder  = net::TaggedEncoder<net::Codec, Requests>;
using ResponseEncoder = net::TaggedEncoder<net::Codec, Responses>;
}  // namespace redis

#endif
