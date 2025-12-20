#include "buffer.h"
#include "codec.h"

#include <gtest/gtest.h>
#include <stdexcept>
#include <string>

TEST(BufferTest, SizeAssertions)
{
    Buffer buf{10};

    const char data[6] = "hello";
    buf.append(data, sizeof(data));

    EXPECT_EQ(buf.size(), 6);
}

TEST(BufferTest, StringAssertion)
{
    Buffer buf{10};

    const char data[6] = "hello";
    buf.append(data, sizeof(data));

    char read_data[6];
    buf.cpy(read_data, sizeof(data));
    EXPECT_STREQ(read_data, data);
}

TEST(BufferTest, ConsumeAssertion)
{
    Buffer buf{10};

    const char data[6] = "hello";
    buf.append(data, sizeof(data));

    buf.consume(2);
    EXPECT_EQ(buf.size(), 4);

    char read_data[4];
    buf.cpy(read_data, sizeof(read_data));
    EXPECT_STREQ(read_data, "llo");
}

TEST(BufferTest, WrapAroundAppend)
{
    Buffer buf{10};

    const char data1[6] = "hello";
    buf.append(data1, sizeof(data1));

    buf.consume(6);

    const char data2[7] = "world!";
    buf.append(data2, sizeof(data2));

    EXPECT_EQ(buf.size(), 7);

    char read_data[7];
    buf.cpy(read_data, sizeof(read_data));
    EXPECT_STREQ(read_data, data2);
}

TEST(BufferTest, OutOutMemoryAppend)
{
    Buffer buf{10};

    const char data1[12] = "hello world";
    EXPECT_THROW(buf.append(data1, sizeof(data1)), std::runtime_error);
}
