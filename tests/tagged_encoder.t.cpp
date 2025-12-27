#include <net_buffer.h>
#include <net_codec.h>
#include <net_tagged_encoder.h>

#include <gtest/gtest.h>

#define MAX_BUFFER_SIZE 1024

using MessageTypes = net::Messages<uint32_t, std::string>;
using Encoder      = net::TaggedEncoder<net::Codec, MessageTypes>;

TEST(TaggedEncoderTest, BasicAssertion)
{
    net::Buffer buffer{MAX_BUFFER_SIZE};
    uint32_t    value{32};
    Encoder::write(value, buffer);
    auto deserialized_msg = Encoder::consume_message(buffer);
    ASSERT_TRUE(deserialized_msg.has_value());
    ASSERT_EQ(std::get<uint32_t>(deserialized_msg.value()), value);
}
