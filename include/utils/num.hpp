#pragma once

#include "Token.hpp"
#include "cstdint"

namespace phantom {
  namespace utils {
    enum class NumKind {
      Decimal = 10,
      Hex = 16,
      Octal = 8,
      Binary = 2,
      Invalid
    };

    // Helper functions
    bool starts_with(const std::string& str, const std::string& cmp);
    NumKind numkind(const std::string& str, std::string& log);

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

    uint64_t parse_dec(size_t start, const std::string& str, size_t end, std::string& log);
    uint64_t parse_hex(size_t start, const std::string& str, size_t end, std::string& log);
    uint64_t parse_oct(size_t start, const std::string& str, size_t end, std::string& log);
    uint64_t parse_bin(size_t start, const std::string& str, size_t end, std::string& log);

    uint64_t parse_int(const std::string& str, std::string& log);
    double parse_float(const std::string& str, std::string& log);
  } // namespace numutils
} // namespace phantom
