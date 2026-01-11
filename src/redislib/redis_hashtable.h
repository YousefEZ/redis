#ifndef INCLUDE_REDIS_HASHTABLE_H
#define INCLUDE_REDIS_HASHTABLE_H

#include <cassert>
#include <concepts>
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>

namespace redis {

constexpr std::size_t MAX_SIZE = 1 << 8;  // 1M

using HashCode = uint64_t;

template <typename K>
concept Key = requires(K key)
{
    {std::hash(key)}->std::same_as<HashCode>;
};

template <Key K, typename V>
class Node {
    K                     m_key;
    V                     m_value;
    std::unique_ptr<Node> m_next;
    HashCode              m_hashcode;

  public:
    Node(K key, V value)
    : m_key{std::move(key)}
    , m_value{std::move(value)}
    , m_next{nullptr}
    , m_hashcode{std::hash(m_key)}
    {
    }

    Node(const Node& node)
    : m_key{node.m_key}
    , m_value{node.m_value}
    , m_next{nullptr}
    , m_hashcode{node.m_hashcode}
    {
    }

    uint64_t chain_length() const
    {
        Node*    curr = m_next.get();
        uint64_t size = 1;
        while (curr) {
            ++size;
            curr = curr->m_next.get();
        }
        return size;
    }

    const K& key() const { return m_key; }
    V&       value() const { return m_value; }
    HashCode hashcode() const { return m_hashcode; }
};

template <Key K, typename V>
class HashMapImpl {
    using HashNode = Node<K, V>;

    // power of 2 array size 2^n - 1, used to cover hcode
    std::size_t m_mask = MAX_SIZE - 1;

    // number of keys in the table
    std::size_t m_size = 0;

    std::unique_ptr<std::unique_ptr<HashNode>[]> m_table;

    std::unique_ptr<HashNode>& get_slot_linked_list(uint64_t hcode)
    {
        return m_table[hcode & m_mask];
    }

  public:
    HashMapImpl(std::size_t max_size)
    : m_mask{max_size - 1}
    , m_table{new std::unique_ptr<HashNode>[max_size]}
    {
        assert(max_size > 0 && ((max_size & (max_size - 1)) == 0) &&
               "size must be power of 2");
    }

    void insert(HashNode node)
    {
        std::unique_ptr<HashNode>& head_node = get_slot_linked_list(
            node.hashcode());
        if (head_node == nullptr) {
            head_node = std::make_unique<HashNode>(std::move(node));
            return;
        }

        HashNode* current = head_node.get();
        while (current->m_next) {
            current = current->m_next.get();
        }

        // current is now the tail of LL
        current->m_next = std::make_unique<HashNode>(std::move(node));

        ++m_size;
    }

    void insert(K key, V value) { insert({std::move(key), std::move(value)}); }

    std::optional<std::reference_wrapper<HashNode> > get(uint64_t hcode)
    {
        std::unique_ptr<HashNode>& head_node = get_slot_linked_list(hcode);
        if (head_node == nullptr) {
            return std::nullopt;
        }

        HashNode* current = head_node.get();
        do {
            if (current->m_hcode == hcode) {
                return *current;
            }
            current = current->m_next.get();
        } while ((current->m_next));

        return std::nullopt;
    }

    std::unique_ptr<HashNode> remove(HashCode hcode)
    {
        std::size_t                idx       = hcode & m_mask;
        std::unique_ptr<HashNode>& head_node = m_table[idx];
        if (head_node == nullptr) {
            return nullptr;
        }
        else if (head_node->m_hcode == hcode) {
            // move the head_node (head of slot) out to local
            std::unique_ptr<HashNode> detachedNode = std::move(head_node);
            // set the value of  head slot to what detached->m_next is pointing
            head_node = std::move(detachedNode->m_next);
            // move resets m_next to nullptr
            return detachedNode;
        }

        HashNode* previous = head_node.get();
        HashNode* current  = head_node->m_next.get();

        do {
            if (current->m_hcode == hcode) {
                previous->m_next = std::move(current->m_next);
                // move sets next m_next to nullptr.
                --m_size;
                return current;
            }
            previous = current;
            current  = current->m_next.get();
        } while ((current->m_next));

        return nullptr;
    }
};

template <Key K, typename V>
class HashMap {
    using HashNode = Node<K, V>;

    HashMapImpl<K, V>                 m_black_map;
    std::optional<HashMapImpl<K, V> > m_red_map;

    void resize() {}

  public:
    HashMap(std::size_t size)
    : m_black_map(size)
    {
    }

    std::optional<std::reference_wrapper<V> > get(uint64_t hcode)
    {
        if (auto node = m_black_map.get(hcode)) {
            return node;
        }

        if (!m_red_map) {
            return std::nullopt;
        }

        if (auto node = m_red_map->remove(hcode)) {
            // optional->get to get underlying of reference_wrapper
            m_black_map.insert(std::move(node->get()));

            if (m_red_map.size() == 0) [[unlikely]] {
                m_red_map.reset();
            }
            return *(m_black_map.get(hcode)).value();
        }

        return std::nullopt;
    }

    std::optional<std::reference_wrapper<HashNode> > insert(K key, V value)
    {
        m_black_map.insert(std::move(key), std::move(value));
    }

    void remove(K key)
    {
        if (!m_black_map.remove(key)) {
            m_red_map.remove(std::move(key));
        }
    }
};

}  // namespace

#endif
