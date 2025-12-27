#include "redis_server.h"

#include <optional>

namespace redis {

std::optional<MessageTypes::MessageVariant>
RedisProcessor::process(MessageTypes::MessageVariant request)
{
    // Placeholder for processing logic
    return request;  // Echo back the request for now
}

}  // namespace redis
