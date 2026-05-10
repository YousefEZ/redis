#ifndef INCLUDE_REDIS_HASHTABLE_H
#define INCLUDE_REDIS_HASHTABLE_H

#include <cassert>
#include <concepts>
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <utility>

namespace redis {

constexpr std::size_t MAX_SIZE = 1 << 8;  // 1M

using HashCode = std::size_t;

template <typename K>
concept Key = requires(K key)
{
    {std::hash<K>{}(key)}->std::convertible_to<HashCode>;
};

template <Key K, typename V>
class HashMapImpl;

template <Key K, typename V>
class Rehasher;

template <Key K, typename V>
class Node {
    K                     m_key;
    V                     m_value;
    std::unique_ptr<Node> m_next;
    HashCode              m_hashcode;
    friend HashMapImpl<K, V>;
    friend Rehasher<K, V>;

  public:
    Node(K key, V value)
    : m_key{std::move(key)}
    , m_value{std::move(value)}
    , m_next{nullptr}
    , m_hashcode{std::hash<K>{}(key)}
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
    V&       value() { return m_value; }
    const V& value() const { return m_value; }
    HashCode hashcode() const { return m_hashcode; }

    operator V&() { return m_value; }

    bool operator==(const Node& other) const;
};

template <Key K, typename V>
bool Node<K, V>::operator==(const Node<K, V>& other) const
{
    return key() == other.key();
}

template <Key K, typename V>
class HashMapImpl {
    using HashNode = Node<K, V>;

    friend Rehasher<K, V>;

    // power of 2 array size 2^n - 1, used to cover hcode
    std::size_t m_mask = MAX_SIZE - 1;

    // number of keys in the table
    std::size_t m_size = 0;

    std::unique_ptr<std::unique_ptr<HashNode>[]> m_table;

    std::unique_ptr<HashNode>& get_slot_linked_list(uint64_t hcode) const
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

    V& insert_or_assign(std::unique_ptr<HashNode> node)
    {
        std::unique_ptr<HashNode>& head_node = get_slot_linked_list(
            node->hashcode());
        if (head_node == nullptr) {
            head_node = std::move(node);
            return head_node->value();
        }
        else if (*head_node == *node) {
            node->m_next = std::move(head_node->m_next);
            head_node    = std::move(node);
            return head_node->value();
        }

        HashNode* previous = head_node.get();
        HashNode* current  = head_node->m_next.get();
        while (current->m_next) {
            if (*current == *node) {
                // the key already exists, so we just need to assign the value
                node->m_next     = std::move(current->m_next);
                previous->m_next = std::move(node);
                return current->value();
            }
            previous = current;
            current  = current->m_next.get();
        }

        // current is now the tail of LL
        current->m_next = std::move(node);

        ++m_size;
        return current->m_next->value();
    }

    V& insert_or_assign(K key, V value)
    {
        return insert_or_assign(
            std::make_unique<HashNode>(std::move(key), std::move(value)));
    }

    std::optional<std::reference_wrapper<HashNode> > get(uint64_t hcode)
    {
        std::unique_ptr<HashNode>& head_node = get_slot_linked_list(hcode);
        if (head_node == nullptr) {
            return std::nullopt;
        }

        HashNode* current = head_node.get();
        do {
            if (current->hashcode() == hcode) {
                return *current;
            }
            current = current->m_next.get();
        } while ((current->m_next));

        return std::nullopt;
    }

    std::optional<std::reference_wrapper<const HashNode> >
    get(uint64_t hcode) const
    {
        std::unique_ptr<HashNode>& head_node = get_slot_linked_list(hcode);
        if (head_node == nullptr) {
            return std::nullopt;
        }

        HashNode const* current = head_node.get();
        do {
            if (current->hashcode() == hcode) {
                return *current;
            }
            current = current->m_next.get();
        } while ((current->m_next));

        return std::nullopt;
    }

    std::unique_ptr<HashNode> detach(const K& key)
    {
        return detach(key, std::hash<K>{}(key));
    }

    std::unique_ptr<HashNode> detach(const K& key, HashCode hcode)
    {
        std::size_t                idx       = hcode & m_mask;
        std::unique_ptr<HashNode>& head_node = m_table[idx];
        if (head_node == nullptr) {
            return nullptr;
        }
        else if (head_node->hashcode() == hcode && head_node->key() == key) {
            // move the state of head_node (head of slot) out to local
            std::unique_ptr<HashNode> detachedNode = std::move(head_node);
            // set the value of head slot to what detached->m_next is pointing
            head_node = std::move(detachedNode->m_next);
            // move resets m_next to nullptr
            return detachedNode;
        }

        HashNode* previous = head_node.get();
        HashNode* current  = head_node->m_next.get();

        do {
            if (current->hashcode() == hcode && head_node->key() == key) {
                std::unique_ptr<HashNode> current = std::move(
                    previous->m_next);
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
class Rehasher {
    using HashNode = HashMapImpl<K, V>::HashNode;
    HashMapImpl<K, V>& m_black_map;
    HashMapImpl<K, V>& m_red_map;

    std::size_t rehash_block(std::size_t idx)
    {
        std::unique_ptr<HashNode>& block = m_red_map.m_table[idx];
        std::size_t                count = 0;
        for (std::size_t count = 0; block != nullptr;
             ++count, --m_red_map.m_size) {
            std::unique_ptr<HashNode> next = std::move(block->m_next);
            m_black_map.insert_or_assign(std::move(block));

            block = std::move(next);
        }
        return count;
    }

  public:
    Rehasher(HashMapImpl<K, V>& black_map, HashMapImpl<K, V>& red_map)
    : m_black_map{black_map}
    , m_red_map{red_map}
    {
    }

    template <std::size_t REHASH_SIZE>
    void rehash()
    {
        std::size_t count = 0;
        for (std::size_t idx = 0;
             count < REHASH_SIZE && idx <= m_red_map.m_mask;
             idx++) {
            count += rehash_block(idx);
        }
    }
};

template <Key K, typename V, std::size_t REHASH_SIZE = 128>
class HashMap {
    using HashNode = Node<K, V>;

    friend Rehasher<K, V>;

    HashMapImpl<K, V>                 m_black_map;
    std::optional<HashMapImpl<K, V> > m_red_map            = std::nullopt;
    uint64_t                          m_migration_position = 0;

    void resize() {}

    // TODO: think about whether the trigger should be implicit or explicit
    void trigger_rehashing()
    {
        if (!m_red_map) {
            return;
        }

        Rehasher{m_black_map, *m_red_map}.template rehash<REHASH_SIZE>();
    }

  public:
    HashMap(std::size_t size)
    : m_black_map(size)
    {
    }

    std::optional<std::reference_wrapper<V> > get(K key)
    {
        HashCode hcode = std::hash<K>{}(key);
        if (auto node = m_black_map.get(hcode)) {
            return node.transform(
                [](const HashNode& node) -> std::reference_wrapper<const V> {
                    return node.value();
                });
        }

        if (!m_red_map) {
            return std::nullopt;
        }

        if (auto node = m_red_map->detach(hcode)) {
            // optional->get to get underlying of reference_wrapper
            m_black_map.insert_or_assign(std::move(node->get()));

            if (m_red_map.size() == 0) [[unlikely]] {
                m_red_map.reset();
            }
            return *(m_black_map.get(hcode)).value();
        }
        trigger_rehashing();

        return std::nullopt;
    }

    std::optional<std::reference_wrapper<const V> > get(K key) const
    {
        HashCode hcode = std::hash<K>{}(key);
        if (auto node = m_black_map.get(hcode)) {
            return node.transform(
                [](const HashNode& node) -> std::reference_wrapper<const V> {
                    return node.value();
                });
        }

        if (!m_red_map) {
            return std::nullopt;
        }

        if (auto node = m_red_map->get(hcode)) {
            return node.transform(
                [](const HashNode& node) -> std::reference_wrapper<const V> {
                    return node.value();
                });
        }
        return std::nullopt;
    }

    std::optional<std::reference_wrapper<V> > insert_or_assign(K key, V value)
    {
        trigger_rehashing();
        auto& mapped_value = m_black_map.insert_or_assign(std::move(key),
                                                          std::move(value));
        if (m_red_map) {
            m_red_map->detach(key);
        }
        return mapped_value;
    }

    void remove(const K& key)
    {
        if (!m_black_map.detach(key) && m_red_map) {
            m_red_map->detach(key);
        }
    }
};

}  // namespace

#endif
