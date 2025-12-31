#ifndef INCLUDED_NET_CODEC_H
#define INCLUDED_NET_CODEC_H

#include <concepts>
#include <cstdint>
#include <optional>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

namespace net {

template <typename T>
struct Codec;

template <typename T>
concept ReadBuffer = requires(T & buffer)
{
    {buffer.size()}->std::same_as<ssize_t>;
    {buffer.capacity()}->std::same_as<ssize_t>;
    {buffer.append(std::declval<const char*>(), std::declval<ssize_t>())}
        ->std::same_as<void>;
    {buffer.cpy(std::declval<void*>(),
                std::declval<ssize_t>(),
                std::declval<ssize_t>())}
        ->std::same_as<void>;
    {buffer.consume(std::declval<ssize_t>())}->std::same_as<void>;
};

template <typename T>
concept WriteBuffer = requires(T & buffer)
{
    {buffer.capacity()}->std::same_as<ssize_t>;
    {buffer.size()}->std::same_as<ssize_t>;
    {buffer.append(std::declval<const char*>(), std::declval<ssize_t>())}
        ->std::same_as<void>;
};

template <typename T, typename BUFFER>
concept Serializable = requires(const T& message, BUFFER& buffer)
{
    {Codec<T>::serialize(message, buffer)}->std::same_as<bool>;
};

template <typename T, typename BUFFER>
concept Deserializable = requires(BUFFER & buffer)
{
    {Codec<T>::deserialize(buffer)}->std::same_as<std::optional<T> >;
};

template <typename T, typename READ_BUFFER, typename WRITE_BUFFER>
concept MessageCodec = Serializable<T, READ_BUFFER> &&
                       Deserializable<T, WRITE_BUFFER>;

/** Format on the wire for a single fixed-length message:
 * *----------------------------------------*
 * |                 data                   |
 * *----------------------------------------*
 */
template <typename T>
requires std::is_scalar_v<T> struct Codec<T> {
    static constexpr bool variable_size = false;

    template <WriteBuffer BUFFER>
    static bool serialize(const T& msg, BUFFER& buffer)
    {
        if (sizeof(T) > buffer.capacity() - buffer.size()) {
            return false;
        }
        buffer.append((char*)&msg, sizeof(T));
        return true;
    }

    template <ReadBuffer BUFFER>
    static std::optional<T> deserialize(BUFFER& buffer)
    {
        if (buffer.size() < sizeof(T)) {
            return {};
        }

        T contents;
        buffer.cpy(&contents, sizeof(T));
        return contents;
    }
};

/** Format on the wire for a single variable length message:
 * *----------*------------------------------------------*
 * | length   |                   data                   |
 * *----------*------------------------------------------*
 */
template <>
struct Codec<std::string> {
    static constexpr bool variable_size = true;
    static uint32_t raw_size(const std::string& msg) { return msg.size(); }

    template <WriteBuffer BUFFER>
    static bool serialize(const std::string& msg, BUFFER& buffer)
    {
        uint32_t length = static_cast<uint32_t>(msg.size());
        if (length + sizeof(uint32_t) > buffer.capacity() - buffer.size()) {
            return false;
        }
        buffer.append((const char*)&length, sizeof(uint32_t));
        buffer.append(msg.c_str(), length);
        return true;
    }

    template <ReadBuffer BUFFER>
    static std::optional<std::string> deserialize(BUFFER& buffer)
    {
        auto buffer_size = buffer.size();
        if (buffer_size < sizeof(uint32_t)) {
            return {};
        }

        uint32_t payload_size;
        buffer.cpy(&payload_size, sizeof(uint32_t));
        if (buffer_size < payload_size + sizeof(uint32_t)) {
            return {};
        }
        buffer.consume(sizeof(uint32_t));

        std::string contents;
        contents.resize_and_overwrite(payload_size,
                                      [&buffer](char* buf, ssize_t size)
                                          noexcept {
                                              buffer.cpy(buf, size);
                                              return size;
                                          });
        buffer.consume(payload_size);
        return contents;
    }
};

/** Format on the wire for vector variable length messages:
 * *-----------*-----*----------*-----*----------*-----*-----*----------*
 * | nentities | len | entity_1 | len | entity_2 | ... | len | entity_n |
 * *-----------*-----*----------*-----*----------*-----*-----*----------*
 *
 *
 * Format on the write for vector fixed size messages:
 * *-----------*----------------*----------------*-----*----------------*
 * | nentities |    entity_1    |    entity_2    | ... |     entity_n   |
 * *-----------*----------------*----------------*-----*----------------*
 */
template <typename T>
struct Codec<std::vector<T> > {
    static uint32_t calculate_total_size(const std::vector<T>& msgs)
        requires(!Codec<T>::variable_size)
    {
        return sizeof(uint32_t) +        // for nentities
               sizeof(T) * msgs.size();  // for each entity
    }

    static uint32_t calculate_total_size(const std::vector<T>& msgs)
        requires(Codec<T>::variable_size)
    {
        uint32_t total_size = sizeof(uint32_t) +               // for nentities
                              sizeof(uint32_t) * msgs.size();  // for each len
        for (const auto& msg : msgs) {
            total_size += Codec<T>::raw_size(msg);  // for each entity_x data
        }
        return total_size;
    }

    template <WriteBuffer BUFFER>
    static bool serialize(const std::vector<T>& msgs, BUFFER& buffer)
    {
        if (calculate_total_size(msgs) > buffer.capacity() - buffer.size()) {
            return false;
        }
        uint32_t nentities = static_cast<uint32_t>(msgs.size());
        buffer.append((const char*)&nentities, sizeof(uint32_t));
        for (const auto& msg : msgs) {
            if (!Codec<T>::serialize(msg, buffer)) [[unlikely]] {
                throw std::runtime_error(
                    "Failed to serialize message in vector");
            }
        }
        return true;
    }

    template <ReadBuffer BUFFER>
    static bool valid_message_in_buffer(const BUFFER& buffer)
    {
        uint32_t nentities;
        buffer.cpy(&nentities, sizeof(uint32_t));

        uint32_t total_size = sizeof(uint32_t);

        uint32_t current_size;
        for (int idx = 0; idx < nentities; ++idx) {
            if (total_size > buffer.size()) [[unlikely]] {
                return false;
            }
            buffer.cpy(&current_size, sizeof(uint32_t), total_size);
            total_size += current_size;
        }
        return total_size > buffer.size();
    }

    template <ReadBuffer BUFFER>
    static std::optional<std::vector<T> > deserialize(BUFFER& buffer)
    {
        if (!valid_message_in_buffer(buffer)) [[unlikely]] {
            return {};
        }
        uint32_t nentities;
        buffer.cpy(&nentities, sizeof(uint32_t));
        buffer.consume(sizeof(uint32_t));
        std::vector<T> entities;
        entities.reserve(nentities);

        for (int i = 0; i < nentities; ++i) {
            entities.emplace_back(std::move(*Codec<T>::deserialize(buffer)));
        }
        return entities;
    }
};
}  // namespace net

#endif
