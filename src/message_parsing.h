#ifndef MESSAGE_PARSING_H
#define MESSAGE_PARSING_H

#include <cstdint>
#include <unistd.h>
#include <vector>

void send_message(int fd, const char *message, uint32_t length);

int receive_message(int fd, std::vector<char> &buffer);

#endif
