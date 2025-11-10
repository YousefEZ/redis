#include "message_parsing.h"
#include "utils.h"

#include <iostream>

void send_message(int fd, const char *message, uint32_t length) {
  ssize_t n = utils::write_full(fd, (char *)&length, sizeof(length));
  if (n != sizeof(length)) {
    // Handle error
    return;
  }

  n = write(fd, message, length);
  if (n != length) {
    // Handle error
    return;
  }
}

int receive_message(int fd, char *buffer) {
  int payload_size;
  ssize_t n = utils::read_full(fd, (char *)&payload_size, sizeof(uint32_t));

  if (n < 0) {
    std::cerr << "[MESSAGE][RECEIVE] unable to grab payload size";
    return -1;
  }

  std::cout << "[MESSAGE][RECEIVE] payload size: " << payload_size << std::endl;

  n = utils::read_full(fd, buffer, payload_size);

  if (n < 0) {
    std::cerr << "[MESSAGE][RECEIVE] read() error";
    return -1;
  }
  return 0;
}
