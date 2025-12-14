#ifndef MESSAGE_PARSING_H
#define MESSAGE_PARSING_H

#include "buffer.h"

#include <concepts>
#include <cstdint>
#include <iostream>
#include <optional>
#include <string>
#include <sys/types.h>
#include <unistd.h>

template <typename T> struct Codec;

template <typename T>
concept Serializable = requires(T &message) {
  {
    Codec<T>::as_bytes(message)
  } -> std::same_as<std::pair<const char *, uint32_t>>;
};

template <typename T>
concept Deserializable = requires(const Buffer &buffer, uint32_t length) {
  { Codec<T>::into(buffer, length) } -> std::same_as<T>;
};

template <typename T>
concept MessageCodec = Serializable<T> && Deserializable<T>;

template <MessageCodec T> struct VariableLengthEncoder {
  using MessageType = T;

  static void write(T message, Buffer &buffer) {
    auto [data, length] = Codec<T>::as_bytes(message);
    buffer.append((const char *)&length, sizeof(uint32_t));
    std::cout << "[MESSAGE][RESPONDING][SENDING] Wrote length of the message: "
              << length << std::endl;
    buffer.append(data, length);
    std::cout << "[MESSAGE][RESPONDING][SENDING] Wrote message of length:"
              << length << std::endl;
  }

  static std::optional<T> consume_message(Buffer &buffer) {
    if (buffer.size() < sizeof(uint32_t)) {
      return {};
    }
    uint32_t payload_size;

    buffer.cpy(&payload_size, sizeof(uint32_t));
    std::cout << "[MESSAGE][PARSING][CONSUME] payload_size=" << payload_size
              << ", buffer_size=" << buffer.size() << std::endl;
    if (buffer.size() >= payload_size + sizeof(uint32_t)) {
      buffer.consume(sizeof(uint32_t));
      T message = Codec<T>::into(buffer, payload_size);
      buffer.consume(payload_size);
      return message;
    }
    return {};
  }
};

template <typename ENCODER>
concept Encoder =
    requires(ENCODER encoder, Buffer &buffer, ENCODER::MessageType message) {
      { ENCODER::write(message, buffer) } -> std::same_as<void>;
      {
        ENCODER::consume_message(buffer)
      } -> std::same_as<std::optional<typename ENCODER::MessageType>>;
    };

template <> struct Codec<std::string> {

  static std::pair<const char *, uint32_t> as_bytes(const std::string &msg) {
    return {msg.c_str(), msg.size()};
  }

  static std::string into(const Buffer &buffer, uint32_t length) {
    std::string contents;
    contents.resize_and_overwrite(
        length, [&buffer](char *buf, std::size_t size) noexcept {
          buffer.cpy(buf, size);
          return size;
        });
    return contents;
  }
};

using StringEncoder = VariableLengthEncoder<std::string>;

#endif
