#include "../include/lexer/lexer.hpp"

namespace phantom {
  char Lexer::peek() const {
    if (index >= source.length())
      return '\0';

    return source[index];
  }

  char Lexer::consume() {
    char this_character = source[index];
    if (new_line(this_character))
      line_number++;

    index++;
    return this_character;
  }

  void Lexer::report_error(const std::string& error) {
    log += std::to_string(line_number) + ": " + error + '\n';
  }

  // used to track line_number in `consume()`
  bool Lexer::new_line(char character) {
    return character == '\n';
  }

  bool Lexer::whitespace(char character) {
    return (character == 9 ||  // '\t'
            character == 10 || //  '\n'
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

  Token Lexer::handle_numerics() {
    std::string lexeme;
    char character = peek();
    bool valid = true;

    // Impossible case, at least for now.
    // if (!isdigit(character))
    //   return;

    lexeme.push_back(consume());

    while (alpha_digit(character = peek()) || character == '_') {
      lexeme.push_back(consume());

      if (!digit(character)) {
        valid = false;
        report_error("Invalid character \'" + std::string(character, 1) + "\'");
      }
    }

    Token token(Location(line_number, index));
    token.set_form(lexeme);

    if (valid) {
      token.set_type(TokenType::INTEGER_LITERAL);
      token.set_value(std::stoll(lexeme));
    } else
      token.set_type(TokenType::INVALID);

    return token;
  }

  Token Lexer::handle_strings() {
    std::string lexeme;

    while (alpha_digit(peek()))
      lexeme.push_back(consume());

    Token token(Location(line_number, index));
    token.set_form(lexeme);

    if (recognize_type(lexeme) != -1)
      token.set_type(TokenType::DATA_TYPE);
    else if (recognize_keyword(lexeme) != -1)
      token.set_type(TokenType::KEYWORD);
    else
      token.set_type(TokenType::IDENTIFIER);

    return token;
  }

  std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;

    std::string lexeme;
    while (true) {
      char character = peek();

      // base case
      if (character == '\0') {
        tokens.push_back(Token(TokenType::EndOfFile, Location(line_number, index)));
        break;
      }

      // whitespace includes new_line
      else if (whitespace(character))
        consume();

      else if (recognize_punctuation(character) != -1)
        tokens.push_back(Token(Token::punctuation_type(character),
                               std::string(1, consume()),
                               Location(line_number, index)));

      else if (digit(character))
        tokens.push_back(handle_numerics());

      else if (alphabet(character) || character == '_')
        tokens.push_back(handle_strings());

      else
        tokens.push_back(Token(TokenType::INVALID,
                               std::string(1, consume()),
                               Location(line_number, index)));
    }

    return tokens;
  }

  std::string Lexer::get_log() { return this->log; }
} // namespace phantom
