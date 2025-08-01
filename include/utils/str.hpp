#pragma once

#include <cstddef>

namespace phantom {
  namespace utils {
    struct Str {
      char* content;
      size_t len;
      size_t cap;
    };

    Str init();
    Str init(size_t cap);
    Str init(const char* s);
    int append(Str* str, const char* buffer);
    int appendf(Str* str, const char* format, ...);
    void dump(Str* str);
  }
}
