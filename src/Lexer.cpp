#include <Lexer.hpp>
#include <Logger.hpp>
#include <common.hpp>
#include <utils/NumUtils.hpp>

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

        // TODO: maybe we can use binary search here
        bool found = false;
        for (const auto& primitive : PrimDataTys) {
          if (lexeme == primitive) {
            tokens.emplace_back(Token::Kind::DataType, lexeme,
                                Location(line_number, column_number));
            found = true;
            break;
          }
        }

        if (found)
          continue;

        // TODO: binary search here too
        found = false;
        for (const auto& [keyword, token_kind] : Keywords) {
          if (lexeme == keyword) {
            tokens.emplace_back(token_kind, Location(line_number, column_number));
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

        while (std::isalnum(peek()) || peek() == '.')
          lexeme += consume();

        std::string log;
        num::NumKind number_kind = num::numkind(lexeme, log);
        Token::Kind token_kind;

        // clang-format off
        switch (number_kind) {
          case num::NumKind::Decimal:    token_kind = num::scan_dec(lexeme, log); break;
          case num::NumKind::Hex:        token_kind = num::scan_hex(lexeme, log); break;
          case num::NumKind::Octal:      token_kind = num::scan_oct(lexeme, log); break;
          case num::NumKind::Binary:     token_kind = num::scan_bin(lexeme, log); break;
          case num::NumKind::Invalid:    token_kind = Token::Kind::Invalid; break;
        }
        // clang-format on

        if (!log.empty())
          logger.log(Logger::Level::ERROR, log, {line_number, column_number - lexeme.length()});

        tokens.emplace_back(token_kind, lexeme, Location(line_number, column_number));
        continue;
      }

      // ponctuations
      bool found = false;
      for (const auto& [pattern, token_kind] : Puncts) {
        std::string str = std::string(pattern);
        if (skip_prefix(str)) {
          tokens.emplace_back(token_kind, Location(line_number, column_number));
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
} // namespace phantom
