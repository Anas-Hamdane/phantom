#pragma once

#include "data.hpp"

namespace phantom {
  namespace irgen {
    enum class ValKind {
      Register,
      Constant
    };

    struct Value {
      ValKind kind; 

      union {
        Register reg;
        Constant cons;
      } data;
    };
  }
}
