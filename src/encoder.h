#ifndef INCLUDED_ENCODER_H
#define INCLUDED_ENCODER_H

#include "buffer.h"

#include <concepts>
#include <optional>

template <typename ENCODER>
concept Encoder =
    requires(ENCODER encoder, Buffer &buffer, ENCODER::MessageType message) {
      { ENCODER::write(message, buffer) } -> std::same_as<void>;
      {
        ENCODER::consume_message(buffer)
      } -> std::same_as<std::optional<typename ENCODER::MessageType>>;
    };

#endif
