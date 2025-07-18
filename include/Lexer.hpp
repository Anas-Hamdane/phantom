#ifndef PHANTOM_LEXER_HPP
#define PHANTOM_LEXER_HPP

#include <Token.hpp>
#include <vector>

namespace phantom {
  class Logger;
  class Lexer {
    const std::string source;
    const Logger& logger;
    size_t line_number;
    size_t line_column;
    size_t index;

    char peek(const off_t offset = 0) const;

    char consume(const off_t offset = 1);

    bool match(const char character, const off_t offset = 0) const;

    constexpr static int recognize_punctuation(const char character);

    constexpr static int recognize_type(const std::string& buffer);

    constexpr static int recognize_keyword(const std::string& buffer);

    bool new_line(char character);

    bool whitespace(char character);

    bool alphabet(char character);

    bool digit(char character);

    bool alpha_digit(char character);

    bool float_suffix(char character);

    bool double_suffixe(char character);

    Token invalid_token(std::string form);

    Token handle_numerics();

    Token handle_words();

    bool escaped(size_t pos);

    Token eat_character();

    Token eat_string();

public:
    explicit Lexer(const std::string& source, const Logger& logger)
      : source(source), logger(logger), line_number(1), line_column(0), index(0) {}

    std::vector<Token> lex();
  };
} // namespace phantom

#endif // !PHANTOM_LEXER_HPP
