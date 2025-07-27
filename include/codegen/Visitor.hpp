#pragma once

#include "data/Function.hpp"
#include <map>

namespace phantom {
  namespace codegen {
    class Visitor {
      std::map<std::string, Function> fns_table;
      std::map<std::string, Variable> vars_table;

  public:

    };
  } // namespace codegen
} // namespace phantom
