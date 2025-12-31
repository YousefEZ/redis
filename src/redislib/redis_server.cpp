#include "redis_server.h"

#include <optional>

namespace redis {

std::optional<KeyEncoder::MessageType>
RedisProcessor::process(KeyEncoder::MessageType request)
{
    // Placeholder for processing logic
    return request;  // Echo back the request for now
}

}  // namespace redis
