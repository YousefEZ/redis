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

struct MissingKeyResponse {};

template <typename T>
struct SetRequest {
    std::string key;
    T           value;
};

struct SetResponse {
    bool success;
};

using TypeValues = meta::TypeList<std::string>;

using SetRequests    = TypeValues::Map<SetRequest>;
using GetResponses   = TypeValues::Map<GetResponse>;
using ErrorResponses = meta::TypeList<MissingKeyResponse>;

using RequestTypeList = SetRequests::WithBackAppend<GetRequest>;
using ResponseTypeList =
    GetResponses::WithBackAppend<SetResponse>::Concatenate<ErrorResponses>;

using Requests  = RequestTypeList::To<net::Messages>;
using Responses = ResponseTypeList::To<net::Messages>;

using RequestEncoder  = net::TaggedEncoder<net::Codec, Requests>;
using ResponseEncoder = net::TaggedEncoder<net::Codec, Responses>;
}  // namespace redis

#endif
