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

  ssize_t read_from(const int fd);

  void append(const char *buf, ssize_t n);

  void consume(ssize_t n);

  void cpy(void *dst, ssize_t n) const;
  std::string cpy(ssize_t n) const;

  ssize_t write_to(const int fd, ssize_t n) const;

  ssize_t size() const;
  bool empty() const;

  void clear() { data_start = data_end = start.get(); }
};

#endif
