#include <utility>

#include "../../include/parser/parameter.hpp"
namespace phantom {
  Parameter::Parameter(std::string name, std::string type)
    : name(std::move(name)), type(std::move(type)) {}
}
