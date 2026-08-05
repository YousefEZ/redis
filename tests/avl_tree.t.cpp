#include "redis_avltree.h"
#include <gtest/gtest.h>
#include <spdlog/spdlog.h>

TEST(AvlTree, Insertion)
{
    redis::AvlTree<int> avl_tree;
    avl_tree.insert(3);
}

TEST(AvlTree, Removal)
{
    redis::AvlTree<int> avl_tree;
    avl_tree.insert(3);
    avl_tree.remove(3);
}
