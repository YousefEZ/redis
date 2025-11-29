#ifndef INCLUDE_BUFFER_H
#define INCLUDE_BUFFER_H

#include <memory>
#include <unistd.h>

class Buffer {
  std::unique_ptr<char[]> start;
  char *end;
  char *data_start;
  char *data_end;

public:
  Buffer(ssize_t size);

  void consume(ssize_t n);
  void append(char *buf, ssize_t n);
  ssize_t write_to(const int fd, ssize_t n) const;

  ssize_t size() const;
  bool empty() const;
};

#endif
