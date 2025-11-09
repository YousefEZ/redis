#include <iostream>

namespace utils {

void die_on(int rc, const char *message) {
  if (rc != 0) {
    std::cerr << "[RC=" << rc << "][ERRNO=" << errno << "] " << message
              << std::endl;
    exit(rc);
  }
}

} // namespace utils
