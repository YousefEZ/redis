#include "tagged_encoder.h"
#include "buffer.h"
#include "codec.h"

#include <gtest/gtest.h>

#define MAX_BUFFER_SIZE 1024

using MessageTypes = Messages<uint32_t, std::string>;
using Encoder      = TaggedEncoder<Codec, MessageTypes>;

TEST(TaggedEncoderTest, BasicAssertion)
{
    Buffer   buffer{MAX_BUFFER_SIZE};
    uint32_t value{32};
    Encoder::write(value, buffer);
    auto deserialized_msg = Encoder::consume_message(buffer);
    ASSERT_TRUE(deserialized_msg.has_value());
    ASSERT_EQ(std::get<uint32_t>(deserialized_msg.value()), value);
}
