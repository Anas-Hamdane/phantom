#include <Lexer.hpp>
#include <common.hpp>
#include <Logger.hpp>

#include <cmath>

namespace phantom {
  std::vector<Token> Lexer::lex() {
    std::vector<Token> tokens;

    while (true) {

      if (match('\0')) {
        tokens.emplace_back(Token::Kind::EndOfFile, Location(line_number, column_number));
        break;
      }

      // one line comment
      if (skip_prefix("//")) {
        skip_until("\n");
        continue;
      }

      // multi line comment
      if (skip_prefix("/*")) {
        skip_until("*/");
        continue;
      }

      // ponctuations
      for (const auto& [pattern, token_kind] : Puncts) {
        if (skip_prefix(std::string(pattern))) {
          tokens.emplace_back(token_kind, Location(line_number, column_number));
          continue;
        }
      }

      // identifier/keyword
      if (identifier_start(peek())) {
        std::string lexeme;
        lexeme += consume();

        while (identifier_valid(peek()))
          lexeme += consume();

        for (const auto& [keyword, token_kind] : Keywords) {
          if (lexeme == keyword) {
            tokens.emplace_back(token_kind, Location(line_number, column_number));
            continue;
          }
        }

        tokens.emplace_back(Token::Kind::Identifier, lexeme, Location(line_number, column_number));
        continue;
      }
    }

    return tokens;
  }

  char Lexer::consume() {
    char character = peek();

    if (character == '\n') {
      line_number++;
      column_number = 0;
    }

    index++;
    column_number++;
    return character;
  }
  char Lexer::peek(const off_t offset) const {
    if ((index + offset) >= source.length())
      return '\0';

    return source[index + offset];
  }
  bool Lexer::match(const char character, const off_t offset) const {
    if ((index + offset) >= source.length())
      return false;

    return (source[index + offset] == character);
  }

  bool Lexer::skip_prefix(const std::string& prefix) {
    if (index + prefix.size() > source.size())
      return false;

    std::string check(source.data() + index, prefix.size());

    if (check == prefix) {
      index += prefix.size();
      return true;
    }

    return false;
  }
  void Lexer::skip_until(const std::string& prefix) {
    while (peek() != '\0' && !skip_prefix(prefix)) {
      consume();
    }
  }

  bool Lexer::identifier_start(const char c) {
    return (isalpha((unsigned char)c) || c == '_');
  }
  bool Lexer::identifier_valid(const char c) {
    return (isalnum((unsigned char)c) || c == '_');
  }

  bool Lexer::starts_with(const std::string& str, const std::string& cmp) {
    return (str.compare(0, cmp.length(), cmp) == 0);
  }
  Lexer::NumKind Lexer::numkind(const std::string& str) {
    if (str.empty())
      logger.log(Logger::Level::ERROR, "Invalid empty number literal\n");

    if (starts_with(str, "0x") || starts_with(str, "0X"))
      return NumKind::Hex;

    else if (starts_with(str, "0o") || starts_with(str, "0O"))
      return NumKind::Octal;

    else if (starts_with(str, "0b") || starts_with(str, "0B"))
      return NumKind::Binary;

    else if (!std::isdigit(str.front()) && str.front() != '.')
      logger.log(Logger::Level::ERROR, "Invalid number literal start: " + str + "\n");

    return NumKind::Decimal;
  }

  uint64_t Lexer::parse_hex(size_t start, const std::string& str, size_t end) {
    if (end > str.length())
      logger.log(Logger::Level::FATAL, "`end` > `str.length()` in function `Lexer::parse_hex()`");

    bool valid = false;
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
      else if (c >= 'A' && c <= 'F')
        digit = c - 'A' + 10;
      else
        logger.log(Logger::Level::ERROR, "Invalid hex digit '" + std::string(1, c) + "' in literal: " + str + "\n");

      if (result > (UINT64_MAX >> 4))
        logger.log(Logger::Level::ERROR, "hex literal overflow: " + str + "\n");

      result = (result << 4) | digit;
      valid = true;
    }

    if (!valid)
      logger.log(Logger::Level::ERROR, "Invalid hex literal: " + str + "\n");

    return result;
  }
  uint64_t Lexer::parse_dec(size_t start, const std::string& str, size_t end) {
    if (end > str.length())
      logger.log(Logger::Level::FATAL, "`end` > `str.length()` in function `Lexer::parse_dec()`");

    const static size_t base = 10;

    bool valid = false;
    uint64_t result = 0;

    for (size_t i = start; i < end; ++i) {
      unsigned char c = str[i];
      // skip separators
      if (c == '\'')
        continue;

      if (!std::isdigit(c))
        logger.log(Logger::Level::ERROR, "Invalid decimal digit '" + std::string(1, c) + "' in literal: " + str + "\n");

      int digit = c - '0';
      if (result > (UINT64_MAX - digit) / base)
        logger.log(Logger::Level::ERROR, "decimal literal overflow: " + str + "\n");

      result = (result * base) + digit;
      valid = true;
    }

    if (!valid)
      logger.log(Logger::Level::ERROR, "Invalid number literal start: " + str + "\n");

    return result;
  }
  uint64_t Lexer::parse_oct(size_t start, const std::string& str, size_t end) {
    if (end > str.length())
      logger.log(Logger::Level::FATAL, "`end` > `str.length()` in function `Lexer::parse_oct()`");

    bool valid = false;
    uint64_t result = 0;

    for (size_t i = start; i < end; ++i) {
      unsigned char c = str[i];
      // skip separators
      if (c == '\'')
        continue;

      if (c < '0' || c > '7')
        logger.log(Logger::Level::ERROR, "Invalid octal digit '" + std::string(1, c) + "' in literal: " + str + "\n");

      int digit = c - '0';
      if (result > (UINT64_MAX >> 3))
        logger.log(Logger::Level::ERROR, "octal literal overflow: " + str + "\n");

      result = (result << 3) | digit;
      valid = true;
    }

    if (!valid)
      logger.log(Logger::Level::ERROR, "Invalid number literal start: " + str + "\n");

    return result;
  }
  uint64_t Lexer::parse_bin(size_t start, const std::string& str, size_t end) {
    if (end > str.length())
      logger.log(Logger::Level::FATAL, "`end` > `str.length()` in function `Lexer::parse_bin()`");

    bool valid = false;
    uint64_t result = 0;

    for (size_t i = start; i < end; ++i) {
      unsigned char c = str[i];
      // skip separators
      if (c == '\'')
        continue;

      if (c != '0' && c != '1')
        logger.log(Logger::Level::ERROR, "Invalid binary digit '" + std::string(1, c) + "' in literal: " + str + "\n");

      if (result > (UINT64_MAX >> 1))
        logger.log(Logger::Level::ERROR, "binary literal overflow: " + str + "\n");

      result = (result << 1) | (c - '0');
      valid = true;
    }

    if (!valid)
      logger.log(Logger::Level::ERROR, "Invalid number literal start: " + str + "\n");

    return result;
  }

  uint64_t Lexer::parse_int(const std::string& str) {
    if (str.empty())
      logger.log(Logger::Level::FATAL, "`str.length()` = 0 in function `Lexer::parse_int()`\n");

    NumKind kind = numkind(str);
    size_t start = 0;

    if (kind != NumKind::Decimal)
      start += 2;

    if ((str.length() - start) == 0)
      logger.log(Logger::Level::ERROR, "Invalid number literal: " + str + "\n");

    // clang-format off
    switch (kind) {
      case NumKind::Decimal: return parse_dec(start, str, str.length());
      case NumKind::Hex:     return parse_hex(start, str, str.length());
      case NumKind::Octal:   return parse_oct(start, str, str.length());
      case NumKind::Binary:  return parse_bin(start, str, str.length());
    }
    // clang-format on
    return 0;
  }
  long double Lexer::parse_float(const std::string& str) {
    if (str.empty())
      logger.log(Logger::Level::ERROR, "Invalid floating point literal: " + str + "\n");

    NumKind kind = numkind(str);
    size_t current_character = (kind != NumKind::Decimal) ? 2 : 0;

    if ((str.length() - current_character) == 0)
      logger.log(Logger::Level::ERROR, "invalid floating point literal: " + str + "\n");

    else if (kind == NumKind::Octal || kind == NumKind::Binary)
      logger.log(Logger::Level::ERROR, "float literals must be either Hex or Decimal: " + str + "\n");

    else if (kind == NumKind::Hex ? !std::isxdigit(str.back()) : !std::isdigit(str.back()))
      logger.log(Logger::Level::ERROR, "Invalid floating point end: " + str + "\n");

    // scientific notation
    char scientific_notation = (kind == NumKind::Decimal) ? 'e' : 'p';

    // search for '.'
    size_t has_dot = str.find('.', current_character);

    // search for the scientific notation
    size_t has_sn = str.find(scientific_notation, current_character);
    if (has_sn == std::string::npos) has_sn = str.find(toupper(scientific_notation), current_character);

    // if there's more than one
    if (has_dot != std::string::npos && str.find('.', has_dot + 1) != std::string::npos)
      logger.log(Logger::Level::ERROR, "Too many '.' in floating point literal: " + str + "\n");

    if (has_sn != std::string::npos && (str.find(scientific_notation, has_sn + 1) != std::string::npos
                                     || str.find(toupper(scientific_notation), has_sn + 1) != std::string::npos))
      logger.log(Logger::Level::ERROR, "Too many scientific notations in floating point literal: " + str + "\n");

    if (has_dot != std::string::npos && has_sn != std::string::npos) {
      if (has_dot + 1 == has_sn)
        logger.log(Logger::Level::ERROR, "Scientific notation can't come after a '.'\n");

      if (has_sn < has_dot)
        logger.log(Logger::Level::ERROR, "Scientific notation can't be before the '.'\n");
    }

    uint64_t integer = 0;
    if (has_dot != 0) {
      size_t integer_end = std::min(std::min(has_dot, has_sn), str.length());

      if (kind == NumKind::Decimal)
        integer = parse_dec(current_character, str, integer_end);
      else if (kind == NumKind::Hex)
        integer = parse_hex(current_character, str, integer_end);

      current_character = integer_end;
    }
    current_character++;

    if (has_dot == std::string::npos && has_sn == std::string::npos)
      return integer;

    uint64_t fraction = 0;
    size_t fraction_size = 0;
    size_t base = (kind == NumKind::Decimal) ? 10 : 16;
    if (has_dot != std::string::npos) {
      size_t fraction_end = std::min(has_sn, str.length());

      size_t end = fraction_end;
      fraction_size = end - current_character;

      if (fraction_size > FP_FRACTION_MD) {
        fraction_size = FP_FRACTION_MD;
        end = fraction_size + current_character;
      }

      if (kind == NumKind::Decimal)
        fraction = parse_dec(current_character, str, end);
      else if (kind == NumKind::Hex)
        fraction = parse_hex(current_character, str, end);

      current_character = fraction_end + 1;
    }

    // doesn't have exponent
    if (current_character >= str.length())
      return (integer + (fraction / pow(base, fraction_size)));

    int64_t exponent = 0;
    size_t exponent_base = (kind == NumKind::Decimal) ? 10 : 2;
    size_t len = str.length();

    bool negative = false;
    if (str[current_character] == '-' || str[current_character] == '+') {
      negative = (str[current_character] == '-');
      current_character++;
    }

    exponent = parse_dec(current_character, str, len);
    exponent *= (negative) ? -1 : 1;

    long double result =
        (integer + (fraction / pow(base, fraction_size))) // mantissa
        * pow(exponent_base, exponent);                   // exponent

    if (!std::isfinite(result))
      logger.log(Logger::Level::ERROR, "floating point literal overflow: " + str + "\n");

    return result;
  }
} // namespace phantom
