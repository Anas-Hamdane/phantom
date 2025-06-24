#ifndef PHANTOM_LEXER_HPP
#define PHANTOM_LEXER_HPP

#include "token.hpp"
#include <string>
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

    void report_error(const std::string& error);

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

    static bool new_line(char character);

    static bool whitespace(char character);

    static bool alphabet(char character);

    static bool digit(char character);

    static bool alpha_digit(char character);

    Token handle_numerics();

    Token handle_strings();

public:
    explicit Lexer(std::string source) : source(std::move(source)), line_number(1), index(0) {}

    std::vector<Token> tokenize();

    std::string get_log();
  };
} // namespace phantom

#endif // !PHANTOM_LEXER_HPP
