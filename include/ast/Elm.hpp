#pragma once

#include <string>
#include <vector>

namespace phantom {
  struct AstElm {
    std::string name;
    std::vector<AstElm> children;
  };
}
