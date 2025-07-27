#pragma once

#include <string>

namespace phantom {
  namespace codegen {
    enum class VarType : char {
      Bool = 'b',
      Char = 'b',
      Short = 'w',
      Int = 'l',
      Long = 'q',

      Half = 2,  // idk
      Float = 4, // idk
      Double = 8 // idk
    };
    struct Variable {
      std::string name;
      VarType type;
      size_t position; // for assembly
    };
  } // namespace codegen
} // namespace phantom
