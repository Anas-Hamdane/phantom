#pragma once

#include "irgen/Program.hpp"
#include <cstddef>

namespace phantom {
  namespace codegen {
    struct Variable {
      ir::Type type;
      // stack index
      size_t offset;
    };
  } // namespace codegen
} // namespace phantom
