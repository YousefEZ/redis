#include "redis_server.h"
#include "redis_schema.h"

#include <optional>
#include <stdexcept>
#include <utility>
#include <variant>

namespace redis {

std::optional<RedisProcessor::Variant>
RedisProcessor::get(std::string key) const
{
    auto it = m_kv_store.find(key);
    if (it == m_kv_store.end()) {
        return {};
    }
    return it->second;
}

ResponseTypeList::To<std::variant>
RedisProcessor::operator()(GetRequest request)
{
    auto response = get(request.key);
    if (!response) {
        // TODO: implement expect or something
        throw std::runtime_error("missing key");
    }

    return std::visit(
        [](auto&& arg) -> ResponseTypeList::To<std::variant> {
            std::cout << "GetResponse with type: "
                      << typeid(std::decay_t<decltype(arg)>).name()
                      << std::endl;
            return GetResponse<std::decay_t<decltype(arg)> >{
                std::forward<decltype(arg)>(arg)};
        },
        std::move(*response));
}

std::optional<ResponseEncoder::MessageType>
RedisProcessor::process(RequestEncoder::MessageType request)
{
    // Placeholder for processing logic
    return std::visit(*this, request);
}

}  // namespace redis
