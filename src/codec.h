#ifndef INCLUDED_CODEC_H
#define INCLUDED_CODEC_H

#include "buffer.h"

#include <concepts>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

template <typename T>
struct Codec;

template <typename T>
concept Serializable = requires(const T& message, Buffer& buffer)
{
    {Codec<T>::serialize(message, buffer)}->std::same_as<bool>;
};

template <typename T>
concept Deserializable = requires(Buffer & buffer)
{
    {Codec<T>::deserialize(buffer)}->std::same_as<std::optional<T> >;
};

template <typename T>
concept MessageCodec = Serializable<T> && Deserializable<T>;

/** Format on the wire for a single variable length message:
 * *----------*------*
 * | length   | data |
 * *----------*------*
 */
template <>
struct Codec<std::string> {
    static uint32_t raw_size(const std::string& msg) { return msg.size(); }

    static bool serialize(const std::string& msg, Buffer& buffer)
    {
        uint32_t length = static_cast<uint32_t>(msg.size());
        if (length + sizeof(uint32_t) > buffer.capacity() - buffer.size()) {
            return false;
        }
        buffer.append((const char*)&length, sizeof(uint32_t));
        buffer.append(msg.c_str(), length);
        return true;
    }

    static std::optional<std::string> deserialize(Buffer& buffer)
    {
        uint32_t payload_size;
        buffer.cpy(&payload_size, sizeof(uint32_t));
        if (buffer.size() < payload_size + sizeof(uint32_t)) {
            return {};
        }
        buffer.consume(sizeof(uint32_t));
        std::string contents;
        contents.resize_and_overwrite(payload_size,
                                      [&buffer](char* buf, std::size_t size)
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
 */
template <typename T>
struct Codec<std::vector<T> > {
    static uint32_t calculate_total_size(const std::vector<T>& msgs)
    {
        uint32_t total_size = sizeof(uint32_t) +
                              sizeof(uint32_t) * msgs.size();  // for nentities
        for (const auto& msg : msgs) {
            total_size += Codec<T>::raw_size(msg);  // for message data
        }
        return total_size;
    }

    static bool serialize(const std::vector<T>& msgs, Buffer& buffer)
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

    static bool valid_message_in_buffer(const Buffer& buffer)
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

    static std::optional<std::vector<T> > deserialize(Buffer& buffer)
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

#endif
