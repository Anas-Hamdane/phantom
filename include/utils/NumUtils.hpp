#pragma once

#include "Logger.hpp"
#include "Token.hpp"
#include "cstdint"

namespace phantom {
  namespace numutils {
    enum class NumKind {
      Decimal,
      Hex,
      Octal,
      Binary,
      Mongolien
    };

    // Helper functions
    bool starts_with(const std::string& str, const std::string& cmp);
    NumKind numkind(const std::string& str, const Logger& logger);

    // scanners
    Token::Kind scan_dec(const std::string& str, const Logger& logger);
    Token::Kind scan_hex(const std::string& str, const Logger& logger);
    Token::Kind scan_oct(const std::string& str, const Logger& logger);
    Token::Kind scan_bin(const std::string& str, const Logger& logger);

    // parsers
    uint64_t parse_dec(const std::string& str, const Logger& logger);
    uint64_t parse_hex(const std::string& str, const Logger& logger);
    uint64_t parse_oct(const std::string& str, const Logger& logger);
    uint64_t parse_bin(const std::string& str, const Logger& logger);

    uint64_t parse_int(const std::string& str, const Logger& logger);
    long double parse_float(const std::string& str, const Logger& logger);
  } // namespace numutils
} // namespace phantom
