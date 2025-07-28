#pragma once

#include <cstddef>

namespace phantom {
  namespace codegen {
    enum class VarType : unsigned int {
      Bool = 1,
      Char = 1,
      Short = 2,
      Int = 4,
      Long = 8,

      Half = 2,
      Float = 4,
      Double = 8
    };
    struct Variable {
      const char* name;
      VarType type;
      bool floating_point = false;

      // stack index
      size_t index;
    };
  } // namespace codegen
} // namespace phantom
