#ifndef INCLUDED_REDIS_SERVER_H
#define INCLUDED_REDIS_SERVER_H

#include "redis_hashtable.h"
#include "redis_meta.h"
#include "redis_schema.h"

#include <net_codec.h>
#include <net_polled_connection.h>
#include <net_server.h>
#include <net_single_type_encoder.h>
#include <net_tagged_encoder.h>

#include <optional>
#include <variant>

namespace redis {

class RedisProcessor {
  private:
    using Variant = TypeValues::To<std::variant>;

    static constexpr std::size_t initial_size = 1024;

    HashMap<std::string, Variant> m_kv_store = HashMap<std::string, Variant>{
        initial_size};

    std::optional<std::reference_wrapper<const Variant> >
    get(const std::string& key) const;

    template <typename T>
    bool set(std::string key, T value)
    {
        std::cout << "Setting key: " << key << " with value: " << value
                  << std::endl;
        return m_kv_store.insert_or_assign(key, value).has_value();
    }

  public:
    RedisProcessor() = default;

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
