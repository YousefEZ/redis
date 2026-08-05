#ifndef INCLUDED_REDIS_AVL_TREE_H
#define INCLUDED_REDIS_AVL_TREE_H

#include <memory>
#include <optional>
#include <utility>

namespace redis {

namespace detail {

// left heavy is -ve and right heavy is +ve
template <typename T>
struct Node {
    std::unique_ptr<Node> m_left;
    std::unique_ptr<Node> m_right;
    T                     m_value;
    char                  m_balance = 0;
};

}  // namespace detail

template <typename T>
class AvlTree {
    std::unique_ptr<detail::Node<T> > m_root;

    const std::optional<std::reference_wrapper<const detail::Node<T> > >
    search(const T& value);

    void insert_node_onto(std::unique_ptr<detail::Node<T> >*  current,
                          std::unique_ptr<detail::Node<T> >&& node);

    unsigned char recursive_insert(std::unique_ptr<detail::Node<T> >&  current,
                                   std::unique_ptr<detail::Node<T> >&& node);

  public:
    AvlTree() = default;

    void insert(T value);
    void remove(const T& value);
};

template <typename T>
const std::optional<std::reference_wrapper<const detail::Node<T> > >
AvlTree<T>::search(const T& value)
{
    std::unique_ptr<detail::Node<T> > node =
        std::make_unique<detail::Node<T> >(nullptr, nullptr, std::move(value));

    std::unique_ptr<detail::Node<T> >* current = m_root;

    while ((*current) != nullptr) {
        if ((*current)->m_value < value) {
            current = &(current->m_left);
        }
        else if ((*current)->m_value > value) {
            current = &(current->m_right);
        }
        else {
            return **current;
        }
    }
    return std::nullopt;
}

template <typename T>
void AvlTree<T>::insert_node_onto(std::unique_ptr<detail::Node<T> >*  current,
                                  std::unique_ptr<detail::Node<T> >&& node)
{
    while ((*current) != nullptr) {
        if ((*current)->m_value < node->m_value) {
            current = &(current->m_left);
        }
        else if ((*current)->m_value > node->m_value) {
            current = &(current->m_right);
        }
        else {
            return;
        }
    }

    *current = std::move(current);
}

static void rebalance()
{
}

template <typename T>
unsigned char
AvlTree<T>::recursive_insert(std::unique_ptr<detail::Node<T> >&  current,
                             std::unique_ptr<detail::Node<T> >&& node)
{
    if (current->m_value < node->m_value) {
        if (current->m_left == nullptr) {
            current->m_left = std::move(node);
            current->m_balance--;
            return static_cast<unsigned char>(current->m_right != nullptr);
        }
        else {
            unsigned char result    = recursive_insert(current->m_left,
                                                       std::move(node));
            unsigned char propagate = result && current->m_balance <= 0;
            current->m_balance -= result;

            if (current->m_left->m_balance == 2 ||
                current->m_left->m_balance == -2) {
                rebalance();
            }

            return propagate;
        }
    }
    else if (current->m_value > node->m_value) {
        if (current->m_left == nullptr) {
            current->m_left = std::move(node);
            current->m_balance--;
            return static_cast<unsigned char>(current->m_right != nullptr);
        }
        else {
            unsigned char result    = recursive_insert(current->m_left,
                                                       std::move(node));
            unsigned char propagate = result && current->m_balance <= 0;
            current->m_balance -= result;
            return propagate;
        }
    }
    return 0;
}

template <typename T>
void AvlTree<T>::insert(T value)
{
    std::unique_ptr<detail::Node<T> > node =
        std::make_unique<detail::Node<T> >(nullptr, nullptr, std::move(value));

    std::unique_ptr<detail::Node<T> >* current = &m_root;
    insert_node_onto(current, std::move(node));
}

template <typename T>
void AvlTree<T>::remove(const T& value)
{
    std::unique_ptr<detail::Node<T> >* current = m_root;

    while ((*current) != nullptr) {
        if ((*current)->m_value < value) {
            current = &(current->m_left);
        }
        else if ((*current)->m_value > value) {
            current = &(current->m_right);
        }
        else {
            (*current)->m_value = (*current)->m_left.m_value;
            (*current)->m_left  = std::move((*current)->m_left->m_left);
            // what do we do with left->right? how to merge on the right?
            // to merge on the right we need to effectively re-insert
            // left->right
            insert_node_onto(&(*current)->m_right,
                             std::move((*current)->m_left->m_right));
        }
    }
}

}  // namespace redis

#endif
