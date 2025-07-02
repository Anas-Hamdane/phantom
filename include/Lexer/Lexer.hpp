#ifndef PHANTOM_LEXER_HPP
#define PHANTOM_LEXER_HPP

#include <Lexer/Token.hpp>

#include <utility>
#include <vector>

namespace phantom {
  class Lexer {
    const std::string source;
    std::string log;
    size_t line_number;
    size_t index;

    char peek() const;

    char consume();

    constexpr static int recognize_punctuation(const char character) {
      int begin = 0;
      int end = punctuation.size() - 1;

      while (begin <= end) {
        const int mid = begin + (end - begin) / 2;

        if (punctuation[mid] == character)
          return mid;

        if (punctuation[mid] < character)
          begin = mid + 1;
        else
          end = mid - 1;
      }

      return -1;
    }

    constexpr static int recognize_type(const std::string& buffer) {
      int begin = 0;
      int end = types.size() - 1;

      while (begin <= end) {
        const int mid = begin + (end - begin) / 2;

        if (types[mid] == buffer)
          return mid;

        if (types[mid] < buffer)
          begin = mid + 1;
        else
          end = mid - 1;
      }

      return -1;
    }

    constexpr static int recognize_keyword(const std::string& buffer) {
      int begin = 0;
      int end = keywords.size() - 1;

      while (begin <= end) {
        const int mid = begin + (end - begin) / 2;

        if (keywords[mid] == buffer)
          return mid;

        if (keywords[mid] < buffer)
          begin = mid + 1;
        else
          end = mid - 1;
      }

      return -1;
    }

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
    explicit Lexer(std::string source) : source(std::move(source)), line_number(1), index(0) {}

    std::vector<Token> lex();
  };
} // namespace phantom

#endif // !PHANTOM_LEXER_HPP
