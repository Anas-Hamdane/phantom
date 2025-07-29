#pragma once

#include <cstddef>
namespace phantom {
  namespace irgen {
    enum class Type {
      Void = 0,

      Char = 1,
      Bool = 1,
      Short = 2,
      Int = 4,
      Long = 8,

      Half = 12,
      Float = 14,
      Double = 18
    };

    struct Register {
      const char* name;
      bool allocated;
      Type type;
    };
    struct Function {
      const char* name;
      Type type;

      Register* params;
      size_t params_size;
    };

    // NOTE: `void` is not allowed
    struct Constant {
      Type type;

      union {
        char character;
        bool boolean;
        short short_int;
        int integer;
        long long_int;

        // NOTE: when adding a `half` type check for ranges in `common.hpp`
        float hfp;
        float fp;
        double dfp;
      } data;
    };
  }
}
