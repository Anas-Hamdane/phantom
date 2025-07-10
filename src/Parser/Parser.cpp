// #include "../../include/parser/parser.hpp"
#include <Parser/Parser.hpp>
#include <global.hpp>

namespace phantom {
  Parser::Parser(std::vector<Token> tokens) : tokens(std::move(tokens)), index(0) {}

  // takes the name to simplify parse_primary()
  std::unique_ptr<Expression> Parser::parse_function_call_expression(const std::string& name) {
    /* Form:
     *   name(arg1, arg2, ...)
     */

    const std::string error_msg = "Incorrect function call, use:\n"
                                  "\"name(arg1, arg2, ...)\"\n"
                                  "Ensure the arguments match the function signature.";

    if (!match(TokenType::OPEN_PARENTHESIS))
      Report(error_msg);

    consume(); // (

    std::vector<std::unique_ptr<Expression>> args;
    while (!match(TokenType::CLOSE_PARENTHESIS)) {
      args.push_back(parse_expression());

      if (match(TokenType::COMMA))
        consume();
    }

    consume(); // )
    return std::make_unique<FnCallExpr>(name, std::move(args));
  }

  std::unique_ptr<Expression> Parser::parse_primary() {
    Token token = consume();
    bool negative = false;

    if (token.type == TokenType::PLUS || token.type == TokenType::MINUS) {
      negative = (token.type == TokenType::MINUS);
      token = consume(); // next token
    }

    switch (token.type) {
      case TokenType::AND:
        if (!match(TokenType::IDENTIFIER))
          Report("Expected identifier after reference operator\n", true);

        return std::make_unique<RefExpr>(std::make_unique<IdentifierExpr>(consume().form));

      case TokenType::STAR: {
        auto expr = parse_primary();
        return std::make_unique<DeRefExpr>(std::move(expr));
      }

      case TokenType::DATA_TYPE:
        if (match(TokenType::EQUAL)) {
          consume(); // =
          auto expr = parse_expression();

          return std::make_unique<DataTypeExpr>(std::move(expr), token.form);
        }

        return std::make_unique<DataTypeExpr>(nullptr, token.form);

      case TokenType::IDENTIFIER:
        if (match(TokenType::OPEN_PARENTHESIS))
          return parse_function_call_expression(token.form);

        // negative identifier expression
        if (negative)
          return std::make_unique<BinOpExpr>(
              std::make_unique<IntLitExpr>("0"),
              TokenType::MINUS,
              std::make_unique<IdentifierExpr>(token.form));

        else
          return std::make_unique<IdentifierExpr>(token.form);

      case TokenType::OPEN_PARENTHESIS: {
        auto expr = parse_expression();

        if (!match(TokenType::CLOSE_PARENTHESIS))
          Report("Expected \")\"", true);

        consume(); // )
        return std::move(expr);
      }

      case TokenType::INTEGER_LITERAL:
        return std::make_unique<IntLitExpr>((negative ? "-" : "") + token.form);

      case TokenType::FLOAT_LITERAL:
        return std::make_unique<FloatLitExpr>((negative ? "-" : "") + token.form);

      case TokenType::CHAR_LITERAL:
        return std::make_unique<CharLitExpr>(token.form.at(0));

      case TokenType::STRING_LITERAL:
        return std::make_unique<StrLitExpr>(token.form);

      case TokenType::KEYWORD:
        if (token.form == "true" || token.form == "false")
          return std::make_unique<BoolLitExpr>(token.form);

      default:
        return nullptr;
    }
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
      Report("Expected \";\" after return statement", true);

    consume(); // ;

    return std::make_unique<ReturnStt>(std::move(expr));
  }

  Variable Parser::parse_param() {
    /*
     * Form:
     *   arg: type
     */

    std::string name;
    std::string type;

    const std::string error_msg = "Incorrect parameter set, use:\n"
                                  "\"arg: type\" followed bye \",\" or \")\".";

    if (!match(TokenType::IDENTIFIER))
      Report(error_msg);

    name = consume().form;

    if (!match(TokenType::COLON))
      Report(error_msg);

    consume(); // ':'

    if (!match(TokenType::DATA_TYPE))
      Report(error_msg);

    type = consume().form;

    if (!match(TokenType::CLOSE_PARENTHESIS)) {
      if (!match(TokenType::COMMA))
        Report(error_msg);

      else
        consume(); // ,
    }

    return {name, type};
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
    std::vector<Variable> params;

    const std::string error_msg = "Incorrect function signature use: "
                                  "\n\"fn name(arg: type) => return_type;\" or:"
                                  "\n\"fn name(arg: type) => return_type {}\" or:"
                                  "\n\"fn name(arg: type); // default return_type = void\" or:"
                                  "\n\"fn name(arg: type) {} // default return_type = void\".";

    consume(); // fn

    if (!match(TokenType::IDENTIFIER))
      Report(error_msg);

    // function name/identifier
    name = consume().form;

    if (!match(TokenType::OPEN_PARENTHESIS))
      Report(error_msg);

    consume(); // '('

    while (peek().type != TokenType::CLOSE_PARENTHESIS)
      params.push_back(parse_param());

    consume(); // ')'

    if (match(TokenType::EQUAL) && match(TokenType::GREATER_THAN, 1)) {
      consume(2); // =>

      if (!match(TokenType::DATA_TYPE))
        Report(error_msg);

      type = consume().form;
    }

    // no type specified
    else if (!(match(TokenType::EQUAL) || match(TokenType::GREATER_THAN, 1)))
      type = "void";

    // incorrect signature
    else
      Report(error_msg);

    // now we have completed the declaration
    auto declaration = std::make_unique<FnDecStt>(name, type, params);

    // just declaration
    if (match(TokenType::SEMI_COLON)) return std::move(declaration);

    if (!match(TokenType::OPEN_CURLY_BRACE))
      Report(error_msg);

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
    std::string type;

    const std::string error_msg = "Incorrect variable declaration, use:\n"
                                  "\"let x = 1; // type is not required\"\n"
                                  "\"let x: int; // type is required\"\n";

    if (!match(TokenType::IDENTIFIER))
      Report(error_msg, true);

    name = consume().form;

    if (!match(TokenType::EQUAL) && !match(TokenType::COLON))
      Report(error_msg, true);

    consume(); // = or :
    auto expr = parse_expression();

    if (!match(TokenType::SEMI_COLON))
      Report(error_msg, true);

    consume(); // ;

    return std::make_unique<VarDecStt>(Variable(name, type), std::move(expr));
  }

  std::unique_ptr<Statement> Parser::parse_keyword() {
    const Token identifier = peek();

    // available keywords
    if (identifier.form == "return")
      return parse_return();
    else if (identifier.form == "fn")
      return parse_function();
    else if (identifier.form == "let")
      return parse_variable_declaration();
    else
      Report("Undefined keyword");

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
      Report("Expected \";\" after function call statement", true);

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

    while (peek().type != limit)
      statements.push_back(parse_statement());

    return std::move(statements);
  }

  Token Parser::peek(const off_t offset) const {
    if ((index + offset) >= tokens.size())
      return {TokenType::EndOfFile, Location(0, 0)};

    return tokens[index + offset];
  }

  Token Parser::consume(const off_t offset) {
    if (index >= tokens.size())
      return {TokenType::EndOfFile, Location(0, 0)};

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
