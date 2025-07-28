#pragma once

#include "Token.hpp"
#include "cstdint"

namespace phantom {
  namespace num {
    enum class NumKind {
      Decimal,
      Hex,
      Octal,
      Binary,
      Invalid
    };

    // Helper functions
    bool starts_with(const std::string& str, const std::string& cmp);
    NumKind numkind(const std::string& str, std::string& logger);

    /*
     * Pattern:
     *   Decimal/Hex: <integer>[.<fraction>][e/E[sign]<exponent>]
     *   Octal/Binary: <integer>
     *
     * NOTE:
     *   `'` is considered as a separator.
     */
    Token::Kind scan_dec(const std::string& str, std::string& log);
    Token::Kind scan_hex(const std::string& str, std::string& log);
    Token::Kind scan_oct(const std::string& str, std::string& log);
    Token::Kind scan_bin(const std::string& str, std::string& log);

    uint64_t parse_dec(const std::string& str, std::string& log);
    uint64_t parse_hex(const std::string& str, std::string& log);
    uint64_t parse_oct(const std::string& str, std::string& log);
    uint64_t parse_bin(const std::string& str, std::string& log);

    uint64_t parse_int(const std::string& str, std::string& log);
    long double parse_float(const std::string& str, std::string& log);
  } // namespace numutils
} // namespace phantom
