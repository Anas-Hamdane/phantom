#ifndef PHANTOM_LEXER_HPP
#define PHANTOM_LEXER_HPP

#include "token.hpp"
#include <string>
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

    constexpr static const int recognize_punctuation(const char character) {
    if (punctuation.empty())
      return -1;

    int begin = 0;
    int end = punctuation.size() - 1;

    while (begin <= end) {
      int mid = begin + (end - begin) / 2;

      if (punctuation[mid] == character)
        return mid;

      if (punctuation[mid] < character)
        begin = mid + 1;
      else
        end = mid - 1;
    }

    return -1;
  }

    constexpr static const int recognize_type(const std::string& buffer) {
      if (types.empty())
        return -1;

      int begin = 0;
      int end = types.size() - 1;

      while (begin <= end) {
        int mid = begin + (end - begin) / 2;

        if (types[mid] == buffer)
          return mid;

        if (types[mid] < buffer)
          begin = mid + 1;
        else
          end = mid - 1;
      }

      return -1;
    }

    constexpr static const int recognize_keyword(const std::string& buffer) {
    if (keywords.empty())
      return -1;

    int begin = 0;
    int end = keywords.size() - 1;

    while (begin <= end) {
      int mid = begin + (end - begin) / 2;

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

    Token handle_numerics();

    Token handle_strings();

public:
    Lexer(std::string source) : source(source), index(0), line_number(1) {}

    std::vector<Token> tokenize();

    std::string get_log();
  };
} // namespace phantom

#endif // !PHANTOM_LEXER_HPP
