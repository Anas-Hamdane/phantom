#include <Lexer.hpp>
#include <Logger.hpp>

namespace phantom {
  char Lexer::peek(const off_t offset) const {
    if ((index + offset) >= source.length())
      return '\0';

    return source[index + offset];
  }

  /*
   * TODO: i think there may be a problem for column_number
   * if we consume +1 characters.
   */
  char Lexer::consume(const off_t offset) {
    char this_character;

    if (index >= source.length())
      this_character = '\0';
    else
      this_character = source[index];

    if (new_line(this_character)) {
      line_number++;
      column_number = 0;
    }

    index += offset;
    column_number += offset;
    return this_character;
  }

  bool Lexer::match(const char character, const off_t offset) const {
    if ((index + offset) >= source.length())
      return false;

    return (source[index + offset] == character);
  }

  constexpr int Lexer::recognize_punctuation(const char character) {
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

  constexpr int Lexer::recognize_type(const std::string& buffer) {
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

  constexpr int Lexer::recognize_keyword(const std::string& buffer) {
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

  // used to track line_number in `consume()`
  bool Lexer::new_line(char character) {
    return character == '\n';
  }

  bool Lexer::whitespace(char character) {
    return (character == 9 ||  // '\t'
            character == 10 || // '\n'
            character == 11 || // '\v'
            character == 12 || // '\f'
            character == 13 || // '\r'
            character == 32);  // ' '
  }

  bool Lexer::alphabet(char character) {
    return (character >= 65 && character <= 90) || // [A-Z]
           (character >= 97 && character <= 122);  // [a-z]
  }

  bool Lexer::digit(char character) {
    return (character >= 48 && character <= 57); // [0-9]
  }

  bool Lexer::alpha_digit(char character) {
    return isalpha(character) || digit(character);
  }

  bool Lexer::float_suffix(char character) {
    return (character == 'f' || character == 'F');
  }

  bool Lexer::double_suffixe(char character) {
    return (character == 'd' || character == 'D');
  }

  Token Lexer::invalid_token(std::string form) {
    return Token(TokenType::INVALID, form, Location(line_number, column_number));
  }

  Token Lexer::handle_numerics() {
    std::string lexeme;
    char character;

    size_t dots = 0;
    size_t suffixes = 0;
    size_t invalid_characters = 0;

    while (alpha_digit(character = peek()) || character == '.') {
      if (character == '.')
        dots++;

      else if (float_suffix(character) || double_suffixe(character))
        suffixes++;

      else if (!digit(character))
        invalid_characters++;

      lexeme.push_back(consume());
    }

    // Checking for errors
    if (invalid_characters != 0) {
      logger.log(Logger::Level::ERROR, "Invalid character in number literal", {line_number, column_number});
      return invalid_token(lexeme);
    }

    if (dots > 1) {
      logger.log(Logger::Level::ERROR, "One '.' allowed in number literal", {line_number, column_number});
      return invalid_token(lexeme);
    }

    if (suffixes > 1) {
      logger.log(Logger::Level::ERROR, "One suffixe allowed in number literal", {line_number, column_number});
      return invalid_token(lexeme);
    }

    else if (suffixes == 1 && (!float_suffix(lexeme.back()) && !double_suffixe(lexeme.back()))) {
      logger.log(Logger::Level::ERROR, "Suffixes only allowed at the end of a number literal", {line_number, column_number});
      return invalid_token(lexeme);
    }

    // Returning the valid token

    // float and double case
    if (dots != 0 || suffixes != 0)
      return Token(TokenType::FLOAT_LITERAL, lexeme, Location(line_number, column_number));

    // int case
    else
      return Token(TokenType::INTEGER_LITERAL, lexeme, Location(line_number, column_number));
  }

  Token Lexer::handle_words() {
    std::string lexeme;
    TokenType type;

    while (alpha_digit(peek()) || peek() == '_')
      lexeme.push_back(consume());

    if (recognize_type(lexeme) != -1)
      type = DATA_TYPE;
    else if (recognize_keyword(lexeme) != -1)
      type = KEYWORD;
    else
      type = IDENTIFIER;

    return Token(type, lexeme, {line_number, column_number});
  }

  bool Lexer::escaped(size_t pos) {
    size_t back_slash_count = 0;
    if (pos == 0) return false;

    for (size_t i = pos - 1; i < source.size(); --i) {
      if (source[i] == '\\')
        back_slash_count++;
      else
        break;

      if (i == 0) break;
    }

    return (back_slash_count % 2 == 1);
  }

  Token Lexer::eat_character() {
    consume(); // '

    std::string lexeme;

    if (peek() == '\\') {
      lexeme.push_back(consume()); // '\'
      lexeme.push_back(consume()); // char
    } else
      lexeme.push_back(consume()); // char

    if (peek() != '\'')
      logger.log(Logger::Level::ERROR, "Expected closing single quote after character literal: got '" + std::string(1, peek()) + "'", {line_number, column_number});

    consume(); // '

    return Token(TokenType::CHAR_LITERAL, lexeme, Location(line_number, column_number));
  }

  Token Lexer::eat_string() {
    consume(); // "

    std::string lexeme;

    while (index < source.size()) {
      if (peek() == '"' && !escaped(index))
        break;

      lexeme.push_back(consume());
    }

    if (peek() != '"')
      logger.log(Logger::Level::ERROR, "Unterminated string literal", {line_number, column_number});

    consume(); // "

    return Token(TokenType::STRING_LITERAL, lexeme, Location(line_number, column_number));
  }

  std::vector<Token> Lexer::lex() {
    std::vector<Token> tokens;

    while (true) {
      char character = peek();

      // base case
      if (character == '\0') {
        tokens.emplace_back(TokenType::ENDOFFILE, Location(line_number, column_number));
        break;
      }

      // line comment
      else if (character == '/' && match('/', 1)) {
        consume(2);
        while (!match('\0') && !match('\n'))
          consume();
      }

      else if (character == '/' && match('*', 1)) {
        consume(2);
        while (!match('\0') && !(match('*') && match('/', 1)))
          consume();

        if (match('\0'))
          logger.log(Logger::Level::WARNING, "Unterminated comment block", {line_number, column_number});

        else
          consume(2); // '*/'
      }

      // whitespace includes new_line
      else if (whitespace(character))
        consume();

      else if (character == '\'')
        tokens.push_back(eat_character());

      else if (character == '\"')
        tokens.push_back(eat_string());

      else if (recognize_punctuation(character) != -1) {
        TokenType type = Token::punctuation_type(character);
        tokens.emplace_back(type, std::string(1, character), Location(line_number, column_number));
        consume();
      }

      else if (digit(character))
        tokens.push_back(handle_numerics());

      else if (alphabet(character) || character == '_')
        tokens.push_back(handle_words());

      else
        tokens.emplace_back(TokenType::INVALID,
                            std::string(1, consume()),
                            Location(line_number, column_number));
    }

    return tokens;
  }
} // namespace phantom
