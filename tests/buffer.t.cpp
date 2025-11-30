#include "buffer.h"

#include <gtest/gtest.h>
#include <stdexcept>
#include <string>

// Demonstrate some basic assertions.
TEST(BufferTest, SizeAssertions) {
  // Expect two strings not to be equal.
  Buffer buf{10};

  const char data[6] = "hello";
  buf.append(data, sizeof(data));

  EXPECT_EQ(buf.size(), 6);
}

TEST(BufferTest, StringAssertion) {
  Buffer buf{10};

  const char data[6] = "hello";
  buf.append(data, sizeof(data));

  std::string str = buf.cpy(sizeof(data));
  EXPECT_STREQ(str.c_str(), data);
}

TEST(BufferTest, ConsumeAssertion) {
  Buffer buf{10};

  const char data[6] = "hello";
  buf.append(data, sizeof(data));

  buf.consume(2);
  EXPECT_EQ(buf.size(), 4);

  std::string str = buf.cpy(buf.size());
  EXPECT_STREQ(str.c_str(), "llo");
}

TEST(BufferTest, WrapAroundAppend) {
  Buffer buf{10};

  const char data1[6] = "hello";
  buf.append(data1, sizeof(data1));

  buf.consume(6);

  const char data2[7] = "world!";
  buf.append(data2, sizeof(data2));

  EXPECT_EQ(buf.size(), 7);

  std::string str = buf.cpy(buf.size());
  EXPECT_STREQ(str.c_str(), "world!");
}

TEST(BufferTest, OutOutMemoryAppend) {
  Buffer buf{10};

  const char data1[12] = "hello world";
  EXPECT_THROW(buf.append(data1, sizeof(data1)), std::runtime_error);
}
