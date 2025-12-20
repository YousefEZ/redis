#ifndef INCLUDED_CODEC_H
#define INCLUDED_CODEC_H

#include "buffer.h"

#include <concepts>
#include <cstdint>
#include <string>

template <typename T>
struct Codec;

template <typename T>
concept Serializable = requires(T & message)
{
    {
        Codec<T>::as_bytes(message)
    } -> std::same_as<std::pair<const char*, uint32_t> >;
};

template <typename T>
concept Deserializable = requires(const Buffer& buffer, uint32_t length)
{
    {
        Codec<T>::into(buffer, length)
    } -> std::same_as<T>;
};

template <typename T>
concept MessageCodec = Serializable<T> && Deserializable<T>;

template <>
struct Codec<std::string> {
    static std::pair<const char*, uint32_t> as_bytes(const std::string& msg)
    {
        return {msg.c_str(), msg.size()};
    }

    static std::string into(const Buffer& buffer, uint32_t length)
    {
        std::string contents;
        contents.resize_and_overwrite(length,
                                      [&buffer](char* buf, std::size_t size)
                                          noexcept {
                                              buffer.cpy(buf, size);
                                              return size;
                                          });
        return contents;
    }
};

#endif
