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
    int append(Str* str, const char* buffer);
    int appendf(Str* str, const char* format, ...);
  }
}
