#include <cmath>
#include <common.hpp>
#include <utils/NumUtils.hpp>

namespace phantom {
  namespace numutils {
    // Helper functions
    bool starts_with(const std::string& str, const std::string& cmp) {
      return (str.compare(0, cmp.length(), cmp) == 0);
    }
    NumKind numkind(const std::string& str, std::string& log) {
      if (str.empty()) {
        log += "Invalid empty number literal";
        return NumKind::Mongolien;
      }

      if (starts_with(str, "0x") || starts_with(str, "0X"))
        return NumKind::Hex;

      else if (starts_with(str, "0o") || starts_with(str, "0O"))
        return NumKind::Octal;

      else if (starts_with(str, "0b") || starts_with(str, "0B"))
        return NumKind::Binary;

      else if (std::isdigit(str.front()))
        return NumKind::Decimal;

      log += "Unrecognized prefix for a number literal";
      return NumKind::Mongolien;
    }

    // scanners
    Token::Kind scan_dec(const std::string& str, std::string& log) {
      enum class Section {
        Integer,
        Fraction,
        Exponent
      };

      size_t len = str.length();
      Section section = Section::Integer;
      size_t section_size = 0;

      Token::Kind kind = Token::Kind::IntLit;

      if (!std::isdigit(str[0]))
        log += "Invalid first digit in decimal literal: " + str;
      else
        section_size++;

      for (size_t i = 1; i < len; i++) {
        unsigned char c = str[i];

        if (c == '\'') {
          if (str[i - 1] == '\'')
            log += "invalid extra ' in decimal literal: " + str + "\n";

          continue;
        }

        if (std::isdigit(c)) {
          section_size++;
          continue;
        }

        if (c == '.') {
          if (section != Section::Integer || section_size == 0)
            log += "Invalid section or section size before '.' in decimal literal: " + str + "\n";

          if (kind == Token::Kind::IntLit)
            kind = Token::Kind::FloatLit;

          section = Section::Fraction;
          section_size = 0;
          continue;
        }

        if (c == 'e' || c == 'E') {
          if (section == Section::Exponent || section_size == 0)
            log += "Invalid section or section size before 'e' in decimal literal: " + str + "\n";

          if (i + 1 >= str.length())
            log += "Invalid decimal literal end: " + str + "\n";

          if (str[i + 1] == '+' || str[i + 1] == '-')
            i++;

          if (kind == Token::Kind::IntLit)
            kind = Token::Kind::FloatLit;

          section = Section::Exponent;
          section_size = 0;
          continue;
        }

        log += "Invalid digit '" + std::string(1, c) + "' in decimal literal: " + str + "\n";
      }

      if (section_size == 0)
        log += "Invalid section before the end of a decimal literal: " + str + "\n";

      if (!log.empty())
        return Token::Kind::Mongolien;

      return kind;
    }
    Token::Kind scan_hex(const std::string& str, std::string& log) {
      enum class Section {
        Integer,
        Fraction,
        Exponent
      };

      size_t len = str.length();
      Section section = Section::Integer;
      size_t section_size = 0;

      Token::Kind kind = Token::Kind::IntLit;

      if (!starts_with(str, "0x") && !starts_with(str, "0X"))
        log += "Expected prefix '0x' for hex literal: " + str + "\n";

      // just prefix
      if (len - 2 == 0)
        log += "Expected \"at least\" one degit after hex literal suffix for: " + str + "\n";

      for (size_t i = 2; i < len; i++) {
        unsigned char c = str[i];

        if (c == '\'') {
          if (str[i - 1] == '\'')
            log += "invalid extra ' in hex literal: " + str + "\n";

          continue;
        }

        if (section == Section::Exponent ? std::isdigit(c) : std::isxdigit(c)) {
          section_size++;
          continue;
        }

        if (c == '.') {
          if (section != Section::Integer || section_size == 0)
            log += "Invalid section or section size before '.' in hex literal: " + str + "\n";

          if (kind == Token::Kind::IntLit)
            kind = Token::Kind::FloatLit;

          section = Section::Fraction;
          section_size = 0;
          continue;
        }

        if (c == 'p' || c == 'P') {
          if (section == Section::Exponent || section_size == 0)
            log += "Invalid section or section size before '.' in hex literal: " + str + "\n";

          if (i + 1 >= str.length())
            log += "Invalid hex literal end: " + str + "\n";

          if (str[i + 1] == '+' || str[i + 1] == '-')
            i++;

          if (kind == Token::Kind::IntLit)
            kind = Token::Kind::FloatLit;

          section = Section::Exponent;
          section_size = 0;
          continue;
        }

        log += "Invalid digit '" + std::string(1, c) + "' in hex literal: " + str + "\n";
      }

      if (section_size == 0)
        log += "Invalid section before the end of a hex literal: " + str + "\n";

      return kind;
    }
    Token::Kind scan_oct(const std::string& str, std::string& log) {
      size_t len = str.length();

      if (!starts_with(str, "0o") && !starts_with(str, "0O"))
        log += "Expected prefix '0o' for octal literal: " + str + "\n";

      // just prefix
      if (len - 2 == 0)
        log += "Expected \"at least\" one degit after octal literal suffix for: " + str + "\n";

      for (size_t i = 2; i < len; i++) {
        unsigned char c = str[i];

        if (c == '\'') {
          if (str[i - 1] == '\'')
            log += "invalid extra ' in octal literal: " + str + "\n";

          continue;
        }

        if (c >= '0' && c <= '7')
          continue;

        log += "Invalid digit '" + std::string(1, c) + "' in octal literal: " + str + "\n";
      }

      return Token::Kind::IntLit;
    }
    Token::Kind scan_bin(const std::string& str, std::string& log) {
      size_t len = str.length();

      if (!starts_with(str, "0b") && !starts_with(str, "0B"))
        log += "Expected prefix '0b' for binary literal: " + str + "\n";

      // just prefix
      if (len - 2 == 0)
        log += "Expected \"at least\" one degit after binary literal suffix for: " + str + "\n";

      for (size_t i = 2; i < len; i++) {
        unsigned char c = str[i];

        if (c == '\'') {
          if (str[i - 1] == '\'')
            log += "invalid extra ' in binary literal: " + str + "\n";

          continue;
        }

        if (c == '0' || c == '1')
          continue;

        log += "Invalid digit '" + std::string(1, c) + "' in binary literal: " + str + "\n";
      }

      return Token::Kind::IntLit;
    }

    // parsers
    uint64_t parse_dec(size_t start, const std::string& str, size_t end, std::string& log) {
      const static size_t base = 10;

      uint64_t result = 0;

      for (size_t i = start; i < end; ++i) {
        unsigned char c = str[i];
        // skip separators
        if (c == '\'')
          continue;

        int digit = c - '0';
        if (result > (UINT64_MAX - digit) / base)
          log += "decimal literal overflow: " + str + "\n";

        result = (result * base) + digit;
      }

      return result;
    }
    uint64_t parse_hex(size_t start, const std::string& str, size_t end, std::string& log) {
      uint64_t result = 0;

      for (size_t i = start; i < end; ++i) {
        unsigned char c = str[i];

        // skip separators
        if (c == '\'')
          continue;

        int digit;

        if (c >= '0' && c <= '9')
          digit = c - '0';
        else if (c >= 'a' && c <= 'f')
          digit = c - 'a' + 10;
        else
          digit = c - 'A' + 10;

        if (result > (UINT64_MAX >> 4))
          log += "hex literal overflow: " + str + "\n";

        result = (result << 4) | digit;
      }

      return result;
    }
    uint64_t parse_oct(size_t start, const std::string& str, size_t end, std::string& log) {
      uint64_t result = 0;

      for (size_t i = start; i < end; ++i) {
        unsigned char c = str[i];
        // skip separators
        if (c == '\'')
          continue;

        int digit = c - '0';
        if (result > (UINT64_MAX >> 3))
          log += "octal literal overflow: " + str + "\n";

        result = (result << 3) | digit;
      }

      return result;
    }
    uint64_t parse_bin(size_t start, const std::string& str, size_t end, std::string& log) {
      uint64_t result = 0;

      for (size_t i = start; i < end; ++i) {
        unsigned char c = str[i];
        // skip separators
        if (c == '\'')
          continue;

        if (result > (UINT64_MAX >> 1))
          log += "binary literal overflow: " + str + "\n";

        result = (result << 1) | (c - '0');
      }

      return result;
    }

    uint64_t parse_int(const std::string& str, std::string& log) {
      NumKind kind = numkind(str, log);
      size_t start = 0;

      if (kind != NumKind::Decimal)
        start += 2;

      // clang-format off
      switch (kind) {
        case NumKind::Decimal:   return parse_dec(start, str, str.length(), log);
        case NumKind::Hex:       return parse_hex(start, str, str.length(), log);
        case NumKind::Octal:     return parse_oct(start, str, str.length(), log);
        case NumKind::Binary:    return parse_bin(start, str, str.length(), log);
        case NumKind::Mongolien: return 0;
      }
      // clang-format on
    }
    long double parse_float(const std::string& str, std::string& log) {
      enum class Section {
        Integer,
        Fraction,
        Exponent
      };

      long double integer = 0;
      long double fraction = 0;
      long double exponent = 0;

      size_t fraction_size = 0;

      NumKind kind = numkind(str, log);
      char scientific_notation = (kind == NumKind::Hex) ? 'p' : 'e';
      size_t current_character = (kind == NumKind::Hex) ? 2 : 0;

      Section section = Section::Integer;
      bool negative = false;
      uint64_t tmp = 0;
      for (size_t i = current_character; i < str.length(); ++i) {
        unsigned char c = str[i];

        if (c == '\'')
          continue;

        if (c == '.') {
          integer = tmp;
          tmp = 0;

          section = Section::Fraction;
          continue;
        }

        if (c == scientific_notation || c == toupper(scientific_notation)) {
          if (str[i + 1] == '-' || str[i + 1] == '+') {
            negative = (str[i + 1] == '-');
            i++;
          }

          // clang-format off
          if (section == Section::Fraction) fraction = tmp;
          else integer = tmp;
          // clang-format on
          tmp = 0;

          section = Section::Exponent;
          continue;
        }

        size_t digit;
        if (c >= '0' && c <= '9')
          digit = c - '0';
        else if (c >= 'a' && c <= 'f')
          digit = c - 'a' + 10;
        else
          digit = c - 'A' + 10;

        if (tmp > (UINT64_MAX - digit) / (uint64_t)kind)
          log += "float literal overflow: " + str + "\n";

        if (fraction_size >= FP_FRACTION_MD)
          continue;

        if (section == Section::Fraction)
          fraction_size++;

        tmp = (tmp * (uint64_t)kind) + digit;
      }

      // clang-format off
      switch (section) {
        case Section::Integer:  integer = tmp;  break;
        case Section::Fraction: fraction = tmp; break;
        case Section::Exponent: exponent = tmp; break;
      }
      // clang-format on

      if (negative) exponent *= -1;
      size_t exponent_base = (kind == NumKind::Hex) ? 2 : 10;

      long double result =
          (integer + (fraction / powl((uint64_t)kind, fraction_size))) // mantissa
          * powl(exponent_base, exponent);                             // exponent

      if (!std::isfinite(result))
        log += "float literal overflow: " + str + "\n";

      return result;
    }
  } // namespace numutils
} // namespace phantom
