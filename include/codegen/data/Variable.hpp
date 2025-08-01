#pragma once

#include "common.hpp"
#include <cstddef>

namespace phantom {
  namespace codegen {
    struct Variable {
      Type type;
      // stack index
      size_t offset;
    };
  } // namespace codegen
} // namespace phantom
