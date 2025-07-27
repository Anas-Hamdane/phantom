#pragma once

#include <vector>
#include "Variable.hpp"

namespace phantom {
  namespace codegen {
    enum class FnType {
      Bool,
      Char,
      Short,
      Int,
      Long,

      Half,
      Float,
      Double,

      Void
    };

    // IDEA: add link name

    struct Function {
      std::string name;
      FnType type;
      std::vector<Variable> args;
    };
  }
} // namespace phantom
