#include <cassert>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <string.h>
#include <utils/str.hpp>
#include <common.hpp>

namespace phantom {
  namespace utils {
    Str init() {
      Str str;

      str.cap = STR_INIT;
      str.content = (char*) malloc(str.cap);

      str.len = 0;
      return str;
    }
    Str init(size_t size) {
      Str str;

      str.cap = size;
      str.content = (char*) malloc(str.cap);

      str.len = 0;
      return str;
    }
    Str init(const char* s) {
      Str str;

      size_t n = strlen(s);
      str.cap = n + 1;

      str.content = (char*) malloc(str.cap);
      sprintf(str.content, "%s", s);

      str.len = n;
      return str;
    }
    int append(Str* str, const char* buffer) {
      size_t n = strlen(buffer);

      size_t needed_space = str->len + n + 1;
      if (needed_space > str->cap) {
        while (needed_space > str->cap)
          str->cap *= 2;

        // sizeof(char) = 1
        str->content = (char*)realloc(str->content, str->cap);
        assert(str->content != NULL && "Stop playing DOOM mf");
      }

      char* dest = str->content + str->len;
      sprintf(dest, "%s", buffer);
      str->len += n;

      return n;
    }
    int appendf(Str* str, const char* format, ...) {
      va_list args;

      va_start(args, format);
      int n = vsnprintf(NULL, 0, format, args);
      va_end(args);

      size_t needed_space = str->len + n + 1;
      if (needed_space > str->cap) {
        while (needed_space > str->cap)
          str->cap *= 2;

        // sizeof(char) = 1
        str->content = (char*)realloc(str->content, str->cap);
        assert(str->content != NULL && "Stop playing DOOM mf");
      }

      char* dest = str->content + str->len;
      va_start(args, format);
      vsnprintf(dest, n + 1, format, args);
      va_end(args);
      str->len += n;

      return n;
    }
    void dump(Str* str) {
      free(str->content);
      str->cap = 0;
      str->len = 0;
    }
  } // namespace str
} // namespace phantom
