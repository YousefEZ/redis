#include "message_parsing.h"
#include "buffer.h"

#include <cstdint>
#include <cstring>
#include <iostream>
#include <optional>

ssize_t send_message(int fd, const char *message, uint32_t length) {
  ssize_t n = write(fd, (const char *)&length, sizeof(uint32_t));
  std::cout << "[MESSAGE][PARSING][SENDING] Wrote length of the message: " << n
            << std::endl;
  n = write(fd, message, length);
  std::cout << "[MESSAGE][PARSING][SENDING] Wrote message of length:" << n
            << std::endl;
  return n;
}

std::optional<std::string> consume_message(Buffer &buffer) {
  if (buffer.size() < sizeof(uint32_t)) {
    return {};
  }
  uint32_t payload_size;

  buffer.cpy(&payload_size, sizeof(uint32_t));
  std::cout << "[MESSAGE][PARSING][CONSUME] payload_size=" << payload_size
            << ", buffer_size=" << buffer.size() << std::endl;
  if (buffer.size() >= payload_size + sizeof(uint32_t)) {
    buffer.consume(sizeof(uint32_t));
    std::string message{buffer.cpy(payload_size)};
    buffer.consume(payload_size);
    return message;
  }
  return {};
}
