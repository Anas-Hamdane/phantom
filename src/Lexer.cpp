#include <Lexer.hpp>
#include <Logger.hpp>
#include <common.hpp>

namespace phantom {
  std::vector<Token> Lexer::lex() {
    std::vector<Token> tokens;

    while (true) {
      if (match('\0')) {
        tokens.emplace_back(Token::Kind::EndOfFile, Location(line_number, column_number));
        break;
      }

      if (std::isspace(peek())) {
        consume();
        continue;
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

      // identifier/keyword
      if (identifier_start(peek())) {
        std::string lexeme;
        lexeme += consume();

        while (identifier_valid(peek()))
          lexeme += consume();

        bool found = false;
        for (const auto& [primitive, token_kind] : PrimDataTys) {
          if (lexeme == primitive) {
            tokens.emplace_back(token_kind, lexeme, Location(line_number, column_number));
            found = true;
            break;
          }
        }

        if (found)
          continue;

        found = false;
        for (const auto& [keyword, token_kind] : Keywords) {
          if (lexeme == keyword) {
            tokens.emplace_back(token_kind, lexeme, Location(line_number, column_number));
            found = true;
            break;
          }
        }

        if (found)
          continue;

        tokens.emplace_back(Token::Kind::Identifier, lexeme, Location(line_number, column_number));
        continue;
      }

      // number literals
      if (std::isdigit(peek())) {
        std::string lexeme;
        lexeme += consume();

        while (std::isalnum(peek()))
          lexeme += consume();

        NumKind number_kind = numkind(lexeme);
        Token::Kind token_kind;

        // clang-format off
        switch (number_kind) {
          case NumKind::Decimal:    token_kind = scan_dec(lexeme); break;
          case NumKind::Hex:        token_kind = scan_hex(lexeme); break;
          case NumKind::Octal:      token_kind = scan_oct(lexeme); break;
          case NumKind::Binary:     token_kind = scan_bin(lexeme); break;
          case NumKind::Mongolien:  token_kind = Token::Kind::Mongolien; break;
        }
        // clang-format on

        tokens.emplace_back(token_kind, lexeme, Location(line_number, column_number));
        continue;
      }

      // ponctuations
      bool found = false;
      for (const auto& [pattern, token_kind] : Puncts) {
        std::string str = std::string(pattern);
        if (skip_prefix(str)) {
          tokens.emplace_back(token_kind, str, Location(line_number, column_number));
          found = true;
          break;
        }
      }

      if (found)
        continue;
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
      return character == '\0';

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
    if (str.empty()) {
      logger.log(Logger::Level::ERROR, "Invalid empty number literal\n");
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

    logger.log(Logger::Level::ERROR, "Unrecognized prefix for a number literal",
               {line_number, column_number - str.length() + 1});
    return NumKind::Mongolien;
  }

  Token::Kind Lexer::invalid_kind(const std::string& msg, Location location) {
    logger.log(Logger::Level::ERROR, msg, location);
    return Token::Kind::Mongolien;
  }

  /*
   * Pattern:
   *   Decimal/Hex: <integer>[.<fraction>][e/E[sign]<exponent>]
   *   Octal/Binary: <integer>
   *
   * NOTE:
   *   `'` is considered as a separator.
   */

  Token::Kind Lexer::scan_dec(const std::string& str) {
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
      return invalid_kind("Invalid first digit in decimal literal: " + str, {line_number, column_number - len});
    else
      section_size++;

    for (size_t i = 1; i < len; i++) {
      unsigned char c = str[i];

      if (c == '\'') {
        if (str[i - 1] == '\'') {
          return invalid_kind("invalid extra ' in decimal literal: " + str,
                              {line_number, column_number - len + i});
        }

        continue;
      }

      if (std::isdigit(c)) {
        section_size++;
        continue;
      }

      if (c == '.') {
        if (section != Section::Integer || section_size == 0) {
          return invalid_kind("Invalid section or section size before '.' in decimal literal: " + str,
                              {line_number, column_number - len + i});
        }

        if (kind == Token::Kind::IntLit)
          kind = Token::Kind::FloatLit;

        section = Section::Fraction;
        section_size = 0;
        continue;
      }

      if (c == 'e' || c == 'E') {
        if (section == Section::Exponent || section_size == 0) {
          return invalid_kind("Invalid section or section size before 'e' in decimal literal: " + str,
                              {line_number, column_number - len + i});
        }

        if (i + 1 >= str.length()) {
          return invalid_kind("Invalid decimal literal end: " + str,
                              {line_number, column_number - len + i});
        }

        if (str[i + 1] == '+' || str[i + 1] == '-')
          i++;

        if (kind == Token::Kind::IntLit)
          kind = Token::Kind::FloatLit;

        section = Section::Exponent;
        section_size = 0;
        continue;
      }

      return invalid_kind("Invalid digit '" + std::string(1, c) + "' in decimal literal: " + str,
                          {line_number, column_number - len + i});
    }

    if (section_size == 0) {
      return invalid_kind("Invalid section before the end of a decimal literal: " + str,
                          {line_number, column_number});
    }

    return kind;
  }
  Token::Kind Lexer::scan_hex(const std::string& str) {
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
      return invalid_kind("Expected prefix '0x' for hex literal: " + str, {line_number, column_number - len + 1});

    // just prefix
    if (len - 2 == 0) {
      return invalid_kind("Expected \"at least\" one degit after hex literal suffix for: " + str,
                          {line_number, column_number - len + 2});
    }

    for (size_t i = 2; i < len; i++) {
      unsigned char c = str[i];

      if (c == '\'') {
        if (str[i - 1] == '\'') {
          return invalid_kind("invalid extra ' in hex literal: " + str,
                              {line_number, column_number - len + i});
        }

        continue;
      }

      if (section == Section::Exponent ? std::isdigit(c) : std::isxdigit(c)) {
        section_size++;
        continue;
      }

      if (c == '.') {
        if (section != Section::Integer || section_size == 0) {
          return invalid_kind("Invalid section or section size before '.' in hex literal: " + str,
                              {line_number, column_number - len + i});
        }

        if (kind == Token::Kind::IntLit)
          kind = Token::Kind::FloatLit;

        section = Section::Fraction;
        section_size = 0;
        continue;
      }

      if (c == 'p' || c == 'P') {
        if (section == Section::Exponent || section_size == 0) {
          return invalid_kind("Invalid section or section size before '.' in hex literal: " + str,
                              {line_number, column_number - len + i});
        }

        if (i + 1 >= str.length()) {
          return invalid_kind("Invalid hex literal end: " + str,
                              {line_number, column_number - len + i});
        }

        if (str[i + 1] == '+' || str[i + 1] == '-')
          i++;

        if (kind == Token::Kind::IntLit)
          kind = Token::Kind::FloatLit;

        section = Section::Exponent;
        section_size = 0;
        continue;
      }

      return invalid_kind("Invalid digit '" + std::string(1, c) + "' in hex literal: " + str,
                          {line_number, column_number - len + i});
    }

    if (section_size == 0) {
      return invalid_kind("Invalid section before the end of a hex literal: " + str,
                          {line_number, column_number});
    }

    return kind;
  }
  Token::Kind Lexer::scan_oct(const std::string& str) {
    size_t len = str.length();

    if (!starts_with(str, "0o") && !starts_with(str, "0O"))
      return invalid_kind("Expected prefix '0o' for octal literal: " + str, {line_number, column_number - len + 1});

    // just prefix
    if (len - 2 == 0) {
      return invalid_kind("Expected \"at least\" one degit after octal literal suffix for: " + str,
                          {line_number, column_number - len + 2});
    }

    for (size_t i = 2; i < len; i++) {
      unsigned char c = str[i];

      if (c == '\'') {
        if (str[i - 1] == '\'') {
          return invalid_kind("invalid extra ' in octal literal: " + str,
                              {line_number, column_number - len + i});
        }

        continue;
      }

      if (c >= '0' && c <= '7')
        continue;

      return invalid_kind("Invalid digit '" + std::string(1, c) + "' in octal literal: " + str,
                          {line_number, column_number - len + i});
    }

    return Token::Kind::IntLit;
  }
  Token::Kind Lexer::scan_bin(const std::string& str) {
    size_t len = str.length();

    if (!starts_with(str, "0b") && !starts_with(str, "0B"))
      return invalid_kind("Expected prefix '0b' for binary literal: " + str, {line_number, column_number - len + 1});

    // just prefix
    if (len - 2 == 0) {
      return invalid_kind("Expected \"at least\" one degit after binary literal suffix for: " + str,
                          {line_number, column_number - len + 2});
    }

    for (size_t i = 2; i < len; i++) {
      unsigned char c = str[i];

      if (c == '\'') {
        if (str[i - 1] == '\'') {
          return invalid_kind("invalid extra ' in binary literal: " + str,
                              {line_number, column_number - len + i});
        }

        continue;
      }

      if (c == '0' || c == '1')
        continue;

      return invalid_kind("Invalid digit '" + std::string(1, c) + "' in binary literal: " + str,
                          {line_number, column_number - len + i});
    }

    return Token::Kind::IntLit;
  }
} // namespace phantom
