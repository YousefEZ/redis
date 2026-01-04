#ifndef INCLUDED_REDIS_SERVER_H
#define INCLUDED_REDIS_SERVER_H

#include "redis_meta.h"
#include "redis_schema.h"

#include <net_codec.h>
#include <net_polled_connection.h>
#include <net_server.h>
#include <net_single_type_encoder.h>
#include <net_tagged_encoder.h>

#include <optional>
#include <unordered_map>
#include <variant>

namespace redis {

class RedisProcessor {
  private:
    using Variant = TypeValues::To<std::variant>;

    std::unordered_map<std::string, Variant> m_kv_store;

    std::optional<Variant> get(std::string key) const;

    template <typename T>
    bool set(std::string key, T value)
    {
        std::cout << "Setting key: " << key << " with value: " << value
                  << std::endl;
        return m_kv_store.insert_or_assign(key, value).second;
    }

  public:
    std::optional<ResponseEncoder::MessageType>
    process(RequestEncoder::MessageType request);

    ResponseTypeList::To<std::variant> operator()(GetRequest request);

    template <typename T>
    ResponseTypeList::To<std::variant> operator()(SetRequest<T> request);
};

using RedisServer =
    net::Server<RequestEncoder, ResponseEncoder, RedisProcessor>;

template <typename T>
ResponseTypeList::To<std::variant>
RedisProcessor::operator()(SetRequest<T> request)
{
    set(request.key, request.value);
    return SetResponse{true};
}

}  // namespace redis

#endif
