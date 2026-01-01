#include "redis_server.h"

#include <optional>

namespace redis {

std::optional<ResponseEncoder::MessageType>
RedisProcessor::process(RequestEncoder::MessageType request)
{
    // Placeholder for processing logic
    return "hello world";  // Echo back the request for now
}

}  // namespace redis
