#include "redis_hashtable.h"
#include <gtest/gtest.h>
#include <spdlog/spdlog.h>

TEST(HashTableTest, Insertion)
{
    redis::HashMap<std::string, int> hashmap{16};
    hashmap.insert_or_assign("one", 1);
}

TEST(HashTableTest, Retrieval)
{
    redis::HashMap<std::string, int> hashmap{16};
    hashmap.insert_or_assign("one", 1);
    SPDLOG_INFO("inserted one");
    hashmap.insert_or_assign("two", 2);
    SPDLOG_INFO("inserted two");

    auto one = hashmap.get("one");
    ASSERT_TRUE(one.has_value());
    EXPECT_EQ(one->get(), 1);

    auto two = hashmap.get("two");
    ASSERT_TRUE(two.has_value());
    EXPECT_EQ(two->get(), 2);

    auto three = hashmap.get("three");
    EXPECT_FALSE(three.has_value());
}
