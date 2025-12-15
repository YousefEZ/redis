#ifndef INCLUDED_SIGNALS_H
#define INCLUDED_SIGNALS_H

#include <cstdint>

enum struct Signals : uint8_t {
  e_NONE = 0x0,
  e_READ = 0x1,
  e_WRITE = 0x2,
  e_CLOSE = 0x4
};

inline Signals operator&(Signals a, Signals b) {
  return static_cast<Signals>(static_cast<uint8_t>(a) &
                              static_cast<uint8_t>(b));
}

inline Signals operator|(Signals a, Signals b) {
  return static_cast<Signals>(static_cast<uint8_t>(a) |
                              static_cast<uint8_t>(b));
}

inline Signals &operator&=(Signals &a, Signals b) {
  a = a & b;
  return a;
}

inline Signals operator~(Signals a) {
  return static_cast<Signals>(~static_cast<uint8_t>(a));
}

inline Signals &operator|=(Signals &a, Signals b) {
  a = a | b;
  return a;
}

#endif
