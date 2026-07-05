#include "net_buffer.h"
#include "net_utils.h"

#include <cctype>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <fmt/format.h>
#include <iterator>
#include <memory>
#include <spdlog/spdlog.h>
#include <unistd.h>

namespace net {

namespace {

std::string hexdump_pretty(const void* data, std::size_t size)
{
    const unsigned char* base = static_cast<const unsigned char*>(data);

    constexpr std::size_t                         BYTES_PER_LINE = 16;
    fmt::memory_buffer                            buf;
    std::back_insert_iterator<fmt::memory_buffer> inserter(buf);
    for (std::size_t offset = 0; offset < size; offset += BYTES_PER_LINE) {
        const unsigned char* addr = base + offset;

        // address and offset
        fmt::format_to(inserter, "{:p} (+{})", (const void*)addr, offset);

        // hex bytes
        for (std::size_t i = 0; i < BYTES_PER_LINE; ++i) {
            if (offset + i < size)
                fmt::format_to(inserter, "{:x} ", addr[i]);
            else
                fmt::format_to(inserter, "   ");

            if (i == 7)
                fmt::format_to(inserter, " ");
        }

        fmt::format_to(inserter, " |");

        // ASCII
        for (std::size_t i = 0; i < BYTES_PER_LINE; ++i) {
            if (offset + i < size) {
                unsigned char c = addr[i];
                fmt::format_to(inserter, "{}", std::isprint(c) ? c : '.');
            }
            else {
                fmt::format_to(inserter, " ");
            }
        }

        fmt::format_to(inserter, "|\n");
    }
    return fmt::to_string(buf);
}

}  // namespace

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
        if (first_read < max_read) {
            SPDLOG_DEBUG("[BUFFER] read_from fd={} rv={}", fd, first_read);
            SPDLOG_DEBUG(hexdump_pretty(data_end - first_read, first_read));
            return first_read;
        }
        // wrap to data_start
        ssize_t second_read = read(fd,
                                   start.get(),
                                   data_start - start.get() - 1);
        if (second_read < 0)
            return second_read;
        data_end = start.get() + second_read;
        SPDLOG_DEBUG("[BUFFER] read_from fd={} rv={}",
                     fd,
                     first_read + second_read);
        return second_read + first_read;
    }

    ssize_t rv = read(fd, data_end, data_start - data_end);
    SPDLOG_DEBUG("[BUFFER] read_from fd={} rv={}", fd, rv);
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

void Buffer::cpy(void* dst, ssize_t n, ssize_t offset) const
{
    char* offset_start_pos = data_start + offset;
    if (offset_start_pos + n > end) {
        ssize_t last_part_size = end - offset_start_pos;
        memcpy(dst, offset_start_pos, last_part_size);
        memcpy(static_cast<char*>(dst) + last_part_size,
               start.get(),
               n - last_part_size);
        return;
    }
    SPDLOG_DEBUG("[BUFFER] cpy n={} offset={}", n, offset);
    SPDLOG_DEBUG(hexdump_pretty(offset_start_pos, n));
    memcpy(dst, offset_start_pos, n);
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

ssize_t Buffer::capacity() const
{
    return end - start.get() - 1;
}

}  // namespace net
