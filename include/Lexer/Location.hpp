#ifndef PHANTOM_LOCATION_HPP
#define PHANTOM_LOCATION_HPP

#include <string>

namespace phantom {

  class Location {
    size_t line;
    size_t column;

public:
    Location(size_t line, size_t column);
    std::string format() const;
  };
} // namespace phantom

#endif // !PHANTOM_LOCATION_HPP
