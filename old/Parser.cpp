#include <Parser.hpp>

namespace phantom {
  std::unique_ptr<Expression> Parser::parse_primary() {
    bool negative = false;

    // check negative values case
    if (peek().type == PLUS || peek().type == MINUS) {
      negative = true;
      consume(); // '+' or '-'
    }

    Token token = consume();

    // literals
    if (token.type == INTEGER_LITERAL)
      return std::make_unique<IntLitExpr>((negative ? "-" : "") + token.form);
    else if (token.type == FLOAT_LITERAL)
      return std::make_unique<FloatLitExpr>((negative ? "-" : "") + token.form);
    else if (token.type == CHAR_LITERAL)
      return std::make_unique<CharLitExpr>(token.form.at(0));
    else if (token.type == STRING_LITERAL)
      return std::make_unique<StrLitExpr>(token.form);

    // array literal
    else if (token.type == OPEN_BRACKET) {
      std::vector<std::unique_ptr<Expression>> elements;
      do {
        if (match(COMMA))
          consume(); // ,

        if (match(CLOSE_PARENTHESIS))
          break;

        elements.push_back(parse_expression());
      } while (match(COMMA));

      if (!match(CLOSE_BRACKET))
        logger.log(Logger::Level::ERROR, "Exprected ']'", peek().location);
      else
        consume(); // ]

      return std::make_unique<ArrLitExpr>(std::move(elements));
    }

    // data type:
    // int = 1, int[5] = [1, 2, 3, 4, 5]
    else if (token.type == DATA_TYPE) {
      std::unique_ptr<Expression> value = nullptr;

      // array handling
      if (match(OPEN_BRACKET)) {
        consume(); // [
        std::unique_ptr<Expression> length = parse_expression();

        if (!match(TokenType::CLOSE_BRACKET))
          logger.log(Logger::Level::ERROR, "Expected ']' after an opening bracket", peek().location);
        else
          consume(); // ]

        if (match(EQUAL)) {
          consume(); // =
          value = parse_expression();
        }

        if (!value && !length)
          logger.log(Logger::Level::ERROR, "Illegal array declaration expression: could not determine array size", Location(peek().location.line, 0));

        return std::make_unique<ArrTypeExpr>(token.form, std::move(length), std::move(value));
      }

      if (match(EQUAL)) {
        consume();
        value = parse_expression();
      }

      return std::make_unique<DataTypeExpr>(token.form, std::move(value));
    }

    // unary stuff
    else if (token.type == AMPERSAND) {
      if (!match(TokenType::IDENTIFIER))
        logger.log(Logger::Level::ERROR, "Expected identifier after reference operator", peek().location);

      return std::make_unique<RefExpr>(std::make_unique<IdeExpr>(token.form));
    } else if (token.type == STAR) {
      auto expr = parse_primary(); // could be an IdeExpr or a BinOpExpr

      return std::make_unique<DeRefExpr>(std::move(expr));
    }

    // other stuff
    // TODO: use UnaryExpr instead of BinOpExpr
    else if (token.type == IDENTIFIER) {
      // function call case
      if (match(TokenType::OPEN_PARENTHESIS)) {
        consume(); // (

        std::vector<std::unique_ptr<Expression>> args;

        do {
          if (match(COMMA))
            consume(); // ,

          if (match(CLOSE_PARENTHESIS))
            break;

          args.push_back(parse_expression());
        } while (match(COMMA));

        if (!match(CLOSE_PARENTHESIS))
          logger.log(Logger::Level::ERROR, "Expected ')' after function call arguments", peek().location);
        else
          consume(); // )

        return std::make_unique<FnCallExpr>(token.form, std::move(args));
      }

      // negative identifer: -x
      if (negative) {
        auto temp = std::make_unique<IntLitExpr>("0");
        auto ide = std::make_unique<IdeExpr>(token.form);

        return std::make_unique<BinOpExpr>(std::move(temp), MINUS, std::move(ide));
      }

      return std::make_unique<IdeExpr>(token.form);
    } else if (token.type == OPEN_PARENTHESIS) {
      auto expr = parse_expression();

      if (!match(TokenType::CLOSE_PARENTHESIS))
        logger.log(Logger::Level::ERROR, "Expected ')'", peek().location);

      consume(); // )
      return std::move(expr);
    } else if (token.type == KEYWORD) {
      if (token.form == "true" || token.form == "false")
        return std::make_unique<BoolLitExpr>(token.form);
    }

    return nullptr;
  }

  // pratt parsing
  std::unique_ptr<Expression> Parser::parse_expression(const int min_prec) {
    auto left = parse_primary();

    while (true) {
      Token op = peek();
      const int prec = precedence(op.type);

      if (prec <= min_prec) break;
      consume();

      int next_min = right_associative(op.type) ? prec : (prec + 1);

      auto right = parse_expression(next_min);
      left = std::make_unique<BinOpExpr>(std::move(left), op.type, std::move(right));
    }

    return std::move(left);
  }

  std::unique_ptr<Statement> Parser::parse_return() {
    /* Form:
     *   return something;
     *
     * where something is a valid integer
     */
    consume(); // return

    auto expr = parse_expression();

    if (!match(TokenType::SEMI_COLON))
      logger.log(Logger::Level::ERROR, "Expected ';' after return statement", peek().location);

    consume(); // ;

    return std::make_unique<ReturnStt>(std::move(expr));
  }

  std::unique_ptr<VarDecStt> Parser::parse_param() {
    /*
     * Form:
     *   arg: type
     */

    std::string name;
    std::string type;

    if (!match(TokenType::IDENTIFIER))
      logger.log(Logger::Level::ERROR, "Expected identifier in parameter expression", peek().location);

    name = consume().form;

    if (!match(TokenType::COLON))
      logger.log(Logger::Level::ERROR, "Expected ':' after parameter identifier", peek().location);

    consume(); // ':'

    if (!match(TokenType::DATA_TYPE))
      logger.log(Logger::Level::ERROR, "Expected identifier data type after ':'", peek().location);

    type = consume().form;

    if (!match(TokenType::CLOSE_PARENTHESIS)) {
      if (!match(TokenType::COMMA))
        logger.log(Logger::Level::ERROR, "Expected ',' or ')' after parameter declaration", peek().location);

      else
        consume(); // ,
    }

    return std::make_unique<VarDecStt>(Variable(name), std::make_unique<DataTypeExpr>(type, nullptr));
  }

  std::unique_ptr<Statement> Parser::parse_function() {
    /*
     * Forms:
     *   fn name(arg: type) => return_type;
     *   fn name(arg: type) => return_type {}
     *   fn name(arg: type); // default return_type = void
     *   fn name(arg: type) {} // default return_type = void
     */

    std::string name;
    std::string type;
    std::vector<std::unique_ptr<VarDecStt>> params;

    const std::string error_msg = "Incorrect function signature use: "
                                  "\n\"fn name(arg: type) => return_type;\" or:"
                                  "\n\"fn name(arg: type) => return_type {}\" or:"
                                  "\n\"fn name(arg: type); // default return_type = void\" or:"
                                  "\n\"fn name(arg: type) {} // default return_type = void\".";

    consume(); // fn

    if (!match(TokenType::IDENTIFIER))
      logger.log(Logger::Level::ERROR, "Expected function identifier after 'fn' keyword", peek().location);

    // function name/identifier
    name = consume().form;

    if (!match(TokenType::OPEN_PARENTHESIS))
      logger.log(Logger::Level::ERROR, "Expected '(' after function identifier '" + name + "'", peek().location);

    consume(); // '('

    while (peek().type != TokenType::CLOSE_PARENTHESIS)
      params.push_back(std::move(parse_param()));

    consume(); // ')'

    if (match(TokenType::EQUAL) && match(TokenType::GREATER_THAN, 1)) {
      consume(2); // =>

      if (!match(TokenType::DATA_TYPE))
        logger.log(Logger::Level::ERROR, "Expected function data type after \"=>\" for function '" + name + "'", peek().location);

      type = consume().form;
    }

    // no type specified
    else if (!(match(TokenType::EQUAL) || match(TokenType::GREATER_THAN, 1)))
      type = "void";

    // incorrect signature
    else
      logger.log(Logger::Level::ERROR, error_msg, peek().location);

    // now we have completed the declaration
    auto declaration = std::make_unique<FnDecStt>(name, type, std::move(params));

    // just declaration
    if (match(TokenType::SEMI_COLON)) {
      consume(); // ;
      return std::move(declaration);
    }

    if (!match(TokenType::OPEN_CURLY_BRACE))
      logger.log(Logger::Level::ERROR, error_msg, peek().location);

    // otherwise it is a function definition
    consume(); // '{'
    std::vector<std::unique_ptr<Statement>> body = parse(TokenType::CLOSE_CURLY_BRACE);
    consume(); // '}'

    return std::make_unique<FnDefStt>(std::move(declaration), std::move(body));
  }

  std::unique_ptr<Statement> Parser::parse_variable_declaration() {
    /*
     * Forms:
     *   let x = 1; // type is not required (automatically detected)
     *   let x: int; // type is required (can't be detected)
     */
    consume(); // let

    std::string name;

    if (!match(TokenType::IDENTIFIER))
      logger.log(Logger::Level::ERROR, "Expected identifer in variable declaration expression", peek().location);

    name = consume().form;

    if (!match(TokenType::EQUAL) && !match(TokenType::COLON))
      logger.log(Logger::Level::ERROR, "Expected '=' or ':' in variable declaration expression after identifier '" + name + "'", peek().location);

    consume(); // = or :
    auto expr = parse_expression();

    if (!match(TokenType::SEMI_COLON))
      logger.log(Logger::Level::ERROR, "Expected ';' after variable declaration expression", peek().location);

    consume(); // ;

    return std::make_unique<VarDecStt>(Variable(name), std::move(expr));
  }

  std::unique_ptr<Statement> Parser::parse_keyword() {
    const Token keyword = peek();

    // available keywords
    if (keyword.form == "return")
      return parse_return();
    else if (keyword.form == "fn")
      return parse_function();
    else if (keyword.form == "let")
      return parse_variable_declaration();
    else
      logger.log(Logger::Level::FATAL, "Undefined keyword '" + keyword.form + "'", keyword.location);

    return nullptr;
  }

  std::unique_ptr<Statement> Parser::parse_expr_stt() {
    /* Forms:
     *   function call: name(args);
     *   assignment statement: name = value;
     */

    // assignment/function call are expressions and statements at the same time
    auto expr = parse_expression();

    if (!match(TokenType::SEMI_COLON))
      logger.log(Logger::Level::ERROR, "Expected ';' after an expression statement", peek().location);

    consume(); // ;

    return std::make_unique<ExprStt>(std::move(expr));
  }

  std::unique_ptr<Statement> Parser::parse_statement() {
    switch (peek().type) {
      case TokenType::KEYWORD:
        return parse_keyword();

      case TokenType::IDENTIFIER:
      case TokenType::STAR:
        return parse_expr_stt();

      default:
        consume(); // advance one token to avoid endless loops
        return nullptr;
    };
  }

  std::vector<std::unique_ptr<Statement>> Parser::parse(const TokenType limit) {
    std::vector<std::unique_ptr<Statement>> statements;

    while (peek().type != limit) {
      if (auto stt = parse_statement())
        statements.push_back(std::move(stt));
    }

    return std::move(statements);
  }

  int Parser::precedence(const TokenType type) {
    switch (type) {
      case TokenType::EQUAL:
        return 5;
      case TokenType::PLUS:
      case TokenType::MINUS:
        return 10;
      case TokenType::STAR:
      case TokenType::SLASH:
        return 20;
      default:
        return 0;
    }
  }

  bool Parser::right_associative(const TokenType type) {
    return type == TokenType::EQUAL;
  }

  Token Parser::peek(const off_t offset) const {
    if ((index + offset) >= tokens.size())
      return Token(TokenType::ENDOFFILE, Location(0, 0));

    return tokens[index + offset];
  }

  Token Parser::consume(const off_t offset) {
    if (index >= tokens.size())
      return Token(TokenType::ENDOFFILE, Location(0, 0));

    Token token = tokens[index];

    index += offset;
    return token;
  }

  bool Parser::match(const TokenType token, const off_t offset) const {
    if ((index + offset) >= tokens.size())
      return false;

    return (tokens[index + offset].type == token);
  }
} // namespace phantom
