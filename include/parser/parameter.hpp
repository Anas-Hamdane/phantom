#ifndef PHANTOM_PARAMETER_HPP
#define PHANTOM_PARAMETER_HPP

#include <string>

namespace phantom {
  class Parameter {
    std::string name;
    std::string type;

    public:
    Parameter(std::string name, std::string type);
  };
} // namespace phantom

#endif // !PHANTOM_PARAMETER_HPP
