#include "message_parsing.h"

#include <cstdint>
#include <cstring>
#include <iostream>
#include <optional>
#include <vector>

ssize_t send_message(int fd, const char *message, uint32_t length) {
  ssize_t n = write(fd, (const char *)&length, sizeof(uint32_t));
  std::cout << "[MESSAGE][PARSING][SENDING] Wrote length of the message: " << n
            << std::endl;
  n = write(fd, message, length);
  std::cout << "[MESSAGE][PARSING][SENDING] Wrote message of length:" << n
            << std::endl;
  return n;
}

std::optional<std::string> consume_message(std::vector<char> &buffer) {
  if (buffer.size() < sizeof(uint32_t)) {
    return {};
  }
  uint32_t payload_size;
  memcpy(&payload_size, buffer.data(), sizeof(uint32_t));
  std::cout << "[MESSAGE][PARSING][CONSUME] payload_size=" << payload_size
            << std::endl;
  std::cout << "[MESSAGE][PARSING][CONSUME] buffer_size=" << buffer.size()
            << std::endl;
  if (buffer.size() >= payload_size + sizeof(uint32_t)) {
    std::string value{buffer.data() + sizeof(uint32_t)};
    buffer.erase(buffer.begin(),
                 buffer.begin() + sizeof(uint32_t) + payload_size);
    return value;
  }
  return {};
}

int receive_message(int fd, std::vector<char> &buffer) {
  char buf[64 * 1024];

  ssize_t rv = read(fd, buf, sizeof(buf));
  if (rv < 0) {
    return -1;
  }
  std::cout << "[MESSAGE][PARSING][RECEIVE] received " << rv << " bytes"
            << std::endl;
  buffer.insert(buffer.end(), buf, buf + rv);
  return rv;
}
