#pragma once

#include <string>

namespace phantom {
  namespace codegen {
    enum class VarType {
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
      std::string name;
      VarType type;
      size_t position; // for assembly
    };
  }
}
