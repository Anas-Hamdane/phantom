#include "common.hpp"
#include <cstdio>
#include <cstdlib>

namespace phantom {
  void __todo__impl(const char* file, int line, const char* func) {
    fprintf(stderr, "`todo()` call:\n");
    fprintf(stderr, "feature not yet implemented in function %s at %s:%d", func, file, line);
    std::abort();
  }

  void __unreachable__impl(const char* file, int line, const char* func) {
    fprintf(stderr, "`unreachable()` call in function %s at %s:%d \n", func, file, line);
    std::abort();
  }

  void __torevise__impl(const char* file, int line, const char* func) {
    fprintf(stderr, "`torevise()` call:\n");
    fprintf(stderr, "consider revising function %s at %s:%d\n", func, file, line);
    std::abort();
  }
} // namespace phantom
