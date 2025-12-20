#include "buffer.h"
#include "utils.h"

#include <cstring>
#include <iostream>
#include <memory>
#include <unistd.h>

Buffer::Buffer(ssize_t size)
: start(std::make_unique_for_overwrite<char[]>(size))
, end{start.get() + size}
, data_start{start.get()}
, data_end{start.get()}
{
}

ssize_t Buffer::read_from(const int fd)
{
    if (data_start <= data_end) {
        // init case where start is before end
        ssize_t max_read   = end - data_end;
        ssize_t first_read = read(fd, data_end, end - data_end);
        if (first_read < 0)
            return first_read;
        data_end = data_end + first_read;
        if (first_read < max_read)
            return first_read;
        // wrap to data_start
        ssize_t second_read = read(fd,
                                   start.get(),
                                   data_start - start.get() - 1);
        if (second_read < 0)
            return second_read;
        data_end = start.get() + second_read;
        return second_read + first_read;
    }

    ssize_t rv = read(fd, data_end, data_start - data_end);
    std::cout << "[BUFFER] read_from fd=" << fd << " rv=" << rv << std::endl;
    if (rv >= 0) [[likely]]
        data_end += rv;
    return rv;
}

void Buffer::consume(ssize_t n)
{
    if (data_start + n > data_end) {
        data_start = start.get() + n - (data_end - data_start);
    }
    else {
        data_start = data_start + n;
    }
}

void Buffer::append(const char* buf, ssize_t n)
{
    if (data_end + n > end) {
        ssize_t written_bytes = end - data_end;
        if (start.get() + n - written_bytes > data_start)
            throw std::runtime_error("buffer ran out of memory");
        if (written_bytes > 0)
            memcpy(data_end, buf, written_bytes);
        memcpy(start.get(), buf + written_bytes, n - written_bytes);
        data_end = start.get() + n - written_bytes;
    }
    else {
        memcpy(data_end, buf, n);
        data_end = data_end + n;
    }
}

ssize_t Buffer::write_to(const int fd, ssize_t n) const
{
    if (data_start + n > end) {
        ssize_t rv = utils::write_full(fd, data_start, end - data_start);
        if (rv <= 0)
            return rv;
        rv = utils::write_full(fd, start.get(), n - (end - data_start));
        return rv;
    }

    return utils::write_full(fd, data_start, n);
}

void Buffer::cpy(void* dst, ssize_t n) const
{
    if (data_start + n > end) {
        ssize_t last_part_size = end - data_start;
        memcpy(dst, data_start, last_part_size);
        memcpy(static_cast<char*>(dst) + last_part_size,
               start.get(),
               n - last_part_size);
        return;
    }

    memcpy(dst, data_start, n);
}

ssize_t Buffer::size() const
{
    return data_end < data_start ? end - data_start + data_end - start.get()
                                 : data_end - data_start;
}

bool Buffer::empty() const
{
    return data_end == data_start;
}
