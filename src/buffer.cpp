#include "buffer.h"
#include "utils.h"

#include <cstring>
#include <memory>

Buffer::Buffer(ssize_t size)
    : start(std::make_unique_for_overwrite<char[]>(size)),
      end{start.get() + size}, data_start{start.get()}, data_end{start.get()} {}

void Buffer::consume(ssize_t n) {
  if (data_start + n > data_end) {
    data_start = start.get() + n - (data_end - data_start);
  } else {
    data_start = data_start + n;
  }
}

void Buffer::append(char *buf, ssize_t n) {
  if (data_end + n > end) {
    ssize_t written_bytes = end - data_end;
    if (start.get() + n - written_bytes > data_start)
      throw std::runtime_error("buffer ran out of memory");

    memcpy(data_end, buf, end - data_end);
    memcpy(start.get(), buf + written_bytes, n - written_bytes);
    data_end = start.get() + n - written_bytes;
  } else {
    memcpy(data_end, buf, n);
  }
}

ssize_t Buffer::write_to(const int fd, ssize_t n) const {
  ssize_t rv{};

  if (data_start + n > data_end) {
    rv = utils::write_full(fd, data_start, data_end - data_start);
    if (rv <= 0)
      return rv;
  }

  rv = utils::write_full(fd, data_start, n - rv);
  return rv;
}

ssize_t Buffer::size() const {
  return data_end < data_start ? end - data_start + data_end - start.get()
                               : data_end - data_start;
}

bool Buffer::empty() const { return data_end == data_start; }
