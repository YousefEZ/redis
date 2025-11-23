#ifndef MESSAGE_PARSING_H
#define MESSAGE_PARSING_H

#include <cstdint>
#include <optional>
#include <string>
#include <unistd.h>
#include <vector>

ssize_t send_message(int fd, const char *message, uint32_t length);

std::optional<std::string> consume_message(std::vector<char> &buffer);

int receive_message(int fd, std::vector<char> &buffer);

#endif
