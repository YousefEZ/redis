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

TEST(HashTableTest, Deletion)
{
    redis::HashMap<std::string, int> hashmap{16};
    hashmap.insert_or_assign("one", 1);
    hashmap.insert_or_assign("two", 2);

    hashmap.remove("one");
    auto one = hashmap.get("one");
    EXPECT_FALSE(one);

    auto two = hashmap.get("two");
    ASSERT_TRUE(two.has_value());
    EXPECT_EQ(two->get(), 2);
}

TEST(HashTableTest, Update)
{
    redis::HashMap<std::string, int> hashmap{16};
    hashmap.insert_or_assign("one", 1);
    hashmap.insert_or_assign("one", 10);

    auto one = hashmap.get("one");
    ASSERT_TRUE(one.has_value());
    EXPECT_EQ(one->get(), 10);
}

TEST(HashTableTest, CollisionHandling)
{
    redis::HashMap<std::string, int> hashmap{
        2};  // Small size to force collisions
    hashmap.insert_or_assign("one", 1);
    hashmap.insert_or_assign("two", 2);
    hashmap.insert_or_assign("three", 2);

    auto one = hashmap.get("one");
    ASSERT_TRUE(one.has_value());
    EXPECT_EQ(one->get(), 1);

    auto two = hashmap.get("two");
    ASSERT_TRUE(two.has_value());
    EXPECT_EQ(two->get(), 2);

    auto three = hashmap.get("three");
    ASSERT_TRUE(three.has_value());
    EXPECT_EQ(three->get(), 2);
}

TEST(HashTableTest, Rehashing)
{
    redis::HashMap<std::string, int, 128, 2> hashmap{
        4};  // Small size to trigger rehashing
    for (int i = 0; i < 10; ++i) {
        hashmap.insert_or_assign("key" + std::to_string(i), i);
    }

    for (int i = 0; i < 10; ++i) {
        auto value = hashmap.get("key" + std::to_string(i));
        ASSERT_TRUE(value.has_value());
        EXPECT_EQ(value->get(), i);
    }
}
