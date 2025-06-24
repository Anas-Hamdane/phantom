#include "../include/lexer/location.hpp"

namespace phantom {
  Location::Location(size_t line, size_t column)
      : line(line), column(column) {}

  std::string Location::format() {
    return std::to_string(line) + ":" + std::to_string(column);
  }
} // namespace phantom
