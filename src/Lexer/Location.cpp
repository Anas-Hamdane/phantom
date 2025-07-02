// #include "../../include/lexer/location.hpp"
#include <Lexer/Location.hpp>

namespace phantom {
  Location::Location(size_t line, size_t column)
      : line(line), column(column) {}

  std::string Location::format() const {
    return std::to_string(line) + ":" + std::to_string(column);
  }
} // namespace phantom
