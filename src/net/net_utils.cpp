#include "net_utils.h"
#include <iostream>

namespace utils {

void die_on(int rc, const char* message)
{
    if (rc != 0) {
        std::cerr << "[RC=" << rc << "][ERRNO=" << errno << "] " << message
                  << std::endl;
        exit(rc);
    }
}

int32_t read_full(int fd, char* buf, size_t n)
{
    return detail::action_full(fd, buf, n, read);
}

int32_t write_full(int fd, char* buf, size_t n)
{
    return detail::action_full(fd, buf, n, write);
}

}  // namespace utils
