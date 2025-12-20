#ifndef REDIS_UTILS_H
#define REDIS_UTILS_H

#include <cassert>
#include <cstdint>
#include <unistd.h>

namespace utils {

void die_on(int condition, const char* message);

namespace detail {

template <typename FUNC>
int32_t action_full(int fd, char* buf, size_t n, FUNC action)
{
    while (n > 0) {
        ssize_t rv = action(fd, buf, n);
        if (rv <= 0) {
            return rv;
        }
        assert((size_t)rv <= n);
        n -= (size_t)rv;
        buf += rv;
    }
    return n;
}
}  // namespace detail

int32_t read_full(int fd, char* buf, size_t n);

int32_t write_full(int fd, char* buf, size_t n);

}  // namespace utils

#endif
