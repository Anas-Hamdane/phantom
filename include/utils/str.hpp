#pragma once

#include <cstddef>

namespace phantom {
  namespace str {
    struct Str {
      char* content;
      size_t len;
      size_t capacity;
    };

    Str init();
    Str init(size_t size);
    Str init(const char* s);
    int append(Str* str, const char* buffer);
    int appendf(Str* str, const char* format, ...);
    void dump(Str* str);
  }
}
