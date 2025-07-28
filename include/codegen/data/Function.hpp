#pragma once

#include "Variable.hpp"
#include <vector>

namespace phantom {
  namespace codegen {
    enum class FnType : unsigned int {
      Void = 0,

      Bool = 1,
      Char = 1,
      Short = 2,
      Int = 4,
      Long = 8,

      Half = 2,
      Float = 4,
      Double = 8
    };

    // IDEA: add link name

    struct Function {
      const char* name;
      FnType type;
      std::vector<Variable> args;
      bool terminated = false;
    };
  } // namespace codegen
} // namespace phantom
