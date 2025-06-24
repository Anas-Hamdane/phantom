#include "include/parser.hpp"

namespace phantom {
  Parser::Parser(std::vector<Token> tokens) : tokens(tokens), index(0) {}

  std::unique_ptr<Expression> Parser::parse_primary() {
    Token token = peek();

    consume();
    if (token.type == TokenType::INTEGER_LITERAL)
      return std::make_unique<IntegerLiteralExpression>(IntegerLiteralExpression(token.value));

    else if (token.type == TokenType::IDENTIFIER)
      return std::make_unique<IdentifierExpression>(IdentifierExpression(token.form));

    else if (token.type == TokenType::OPEN_PARENTHESIS) {
      auto expr = parse_expression();

      if (!match(TokenType::CLOSE_PARENTHESIS))
        report_error("Expected \")\"");

      return expr;
    }

    report_error("Invalid primary expression");
    return std::make_unique<Expression>();
  }

  // pratt parsing
  std::unique_ptr<Expression> Parser::parse_expression(int min_prec) {
    auto left = parse_primary();

    while (peek().type != TokenType::SEMI_COLON) {
      Token op = peek();
      int prec = precedence(op.type);

      if (prec < min_prec) break;
      consume();

      auto right = parse_expression(prec + 1);
      left = std::make_unique<BinaryOpExpression>(std::move(left), op.type, std::move(right));
    }

    return left;
  }

  std::unique_ptr<Statement> Parser::parse_keyword() {
    Token identifier = peek();
    consume();

    if (identifier.form == "return") {
      if (!match(TokenType::COLON))
        report_error("Expected \":\" after return keyword");

      consume(); // ':'

      auto expr = parse_expression();

      if (!match(TokenType::SEMI_COLON))
        report_error("Expected \";\" after return statement");

      consume();
      return std::make_unique<ReturnStatement>(std::move(expr));
    }

    else
      return std::make_unique<Statement>();
  }

  std::unique_ptr<Statement> Parser::parse_data_types() {
    std::string type = peek().form;
    consume(); // consume the type

    if (!match(TokenType::COLON))
      report_error("Expected \":\" after variable data type");

    consume(); // consume ':'

    if (!match(TokenType::IDENTIFIER))
      report_error("Expected identifier name after \":\"");

    std::string ident_name = peek().form; // identifier name
    consume();                            // consume the identifier name

    std::unique_ptr<Expression> expr = nullptr;
    if (match(TokenType::EQUAL)) {
      consume(); // consume '='
      expr = parse_expression();
    }

    if (!match(TokenType::SEMI_COLON))
      report_error("Expected \";\" after variable declaration");

    consume(); // ';'

    return std::make_unique<VariableStatement>(ident_name, type, std::move(expr));
  }

  Parameter Parser::parse_param() {
    if (!match(TokenType::DATA_TYPE))
      report_error("Expected data type at the start of a parameter");

    std::string type = peek().form;
    consume();

    if (!match(TokenType::COLON))
      report_error("Expected \":\" after data type");

    consume(); // ':'

    if (!match(TokenType::IDENTIFIER))
      report_error("Expected identifier in variable declaration");

    std::string name = peek().form;
    consume();

    if (!match(TokenType::COMMA) && !match(TokenType::CLOSE_PARENTHESIS))
      report_error("Expected \",\" after parameter declaration");

    if (match(TokenType::COMMA))
      consume(); // ','

    return Parameter(name, type);
  }

  std::unique_ptr<Statement> Parser::parse_function() {
    // function type ex: 'int'
    std::string type = peek().form;
    consume();

    if (peek().form != "fn")
      report_error("Can't parse function, no \"fn\" specifier");

    consume(); // 'fn'

    if (!match(TokenType::COLON))
      report_error("Expected \":\" after \"fn\" specifier");

    consume(); // ':'

    // function name
    std::string name = peek().form;
    consume();

    if (!match(TokenType::OPEN_PARENTHESIS))
      report_error("Expected \"(\" after function identifier");

    consume(); // '('

    std::vector<Parameter> params;
    while (peek().type != TokenType::CLOSE_PARENTHESIS)
      params.push_back(parse_param());

    consume(); // ')'

    std::unique_ptr<FnDecStatement> declaration =
        std::make_unique<FnDecStatement>(name, type, params);
    if (match(TokenType::OPEN_CURLY_BRACE)) {
      consume(); // '{'

      std::vector<std::unique_ptr<Statement>> body;
      body = parse(TokenType::CLOSE_CURLY_BRACE);

      consume(); // '}'

      return std::make_unique<FnDefStatement>(std::move(declaration), std::move(body));
    }

    // else
    if (!match(TokenType::SEMI_COLON))
      report_error("Expected \";\" after function declaration");

    consume(); // ';'

    return std::move(declaration);
  }

  std::unique_ptr<Statement> Parser::parse_statement() {
    TokenType type = peek().type;

    switch (type) {
      case TokenType::KEYWORD:
        return parse_keyword();

      case TokenType::DATA_TYPE:
        if (peek(1).form != "fn")
          return parse_data_types();
        else
          return parse_function();

      default:
        return std::unique_ptr<Statement>();
    };
  }

  std::vector<std::unique_ptr<Statement>> Parser::parse(TokenType limit) {
    std::vector<std::unique_ptr<Statement>> statements;

    while (peek().type != limit)
      statements.push_back(parse_statement());

    return std::move(statements);
  }

  Token Parser::peek(size_t offset) const {
    if ((index + offset) >= tokens.size())
      return Token(TokenType::EndOfFile);

    return tokens[index + offset];
  }

  Token Parser::consume(size_t offset) {
    if ((index + offset) >= tokens.size())
      return Token(TokenType::EndOfFile);

    Token token = tokens[index + offset];

    index += offset;
    return token;
  }

  bool Parser::match(TokenType token, size_t offset) const {
    if ((index + offset) >= tokens.size())
      return false;

    return (tokens[index + offset].type == token);
  }

  void Parser::report_error(const std::string& error) {
    log.append(error + '\n');
  }

  std::string Parser::get_log() { return this->log; }
} // namespace phantom
