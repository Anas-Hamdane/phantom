// #include "../../include/parser/parser.hpp"
#include <global.hpp>
#include <Parser/Parser.hpp>

namespace phantom {
  Parser::Parser(std::vector<Token> tokens) : tokens(std::move(tokens)), index(0) {}

  std::unique_ptr<Expression> Parser::parse_function_call_expression() {
    /* Form:
     *   name(arg1, arg2, ...)
     */
    std::string name;

    const std::string error_msg = "Incorrect function call, use:\n"
                                  "\"name(arg1, arg2, ...)\"\n"
                                  "Ensure the arguments match the function signature.";

    name = consume().form;

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
    const Token token = peek();

    switch (token.type) {
      case TokenType::IDENTIFIER: {
        if (peek(1).type != TokenType::OPEN_PARENTHESIS)
          return std::make_unique<IDExpr>(consume().form);

        else
          return parse_function_call_expression();
      }

      case TokenType::OPEN_PARENTHESIS: {
        consume();
        auto expr = parse_expression();

        if (!match(TokenType::CLOSE_PARENTHESIS))
          Report("Expected \")\"");

        return std::move(expr);
      }

      case TokenType::INTEGER_LITERAL:
        return std::make_unique<IntLitExpr>(consume().form);

      case TokenType::FLOAT_LITERAL:
        return std::make_unique<FloatLitExpr>(consume().form);

      case TokenType::CHAR_LITERAL:
        return std::make_unique<ByteLitExpr>(consume().form.at(0));

      case TokenType::STRING_LITERAL:
        return std::make_unique<StrLitExpr>(consume().form);

      case TokenType::KEYWORD:
        if (token.form == "true" || token.form == "false")
          return std::make_unique<BoolLitExpr>(consume().form);

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
      Report("Expected \";\" after return statement");

    consume(); // ;

    return std::make_unique<ReturnStt>(std::move(expr));
  }

  Parameter Parser::parse_param() {
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
    std::vector<Parameter> params;

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

  std::unique_ptr<Statement> Parser::parse_variable_declaration_let() {
    /*
     * Forms:
     *   let x = 1;
     *   let(type) x = 1; // type is optional
     *   let(type) x; // type is required
     */
    consume(); // let

    std::string name;
    std::string type;

    const std::string error_msg = "Incorrect variable declaration, use:\n"
                                  "\"let name = 1;\"\n"
                                  "\"let(type) name = 1; // type is optional\"\n"
                                  "\"let(type) name; // type is required\"\n";

    // check if type is specified
    if (match(TokenType::OPEN_PARENTHESIS)) {
      consume(); // (

      if (!(match(TokenType::DATA_TYPE) || match(TokenType::CLOSE_PARENTHESIS)))
        Report(error_msg);

      if (match(TokenType::DATA_TYPE))
        type = consume().form;

      if (!match(TokenType::CLOSE_PARENTHESIS))
        Report(error_msg);

      consume(); // )
    }

    if (!match(TokenType::IDENTIFIER))
      Report(error_msg);

    name = consume().form;

    if (match(TokenType::SEMI_COLON)) {
      if (type.empty())
        Report(error_msg);

      return std::make_unique<VarDecStt>(name, type, nullptr);
    }

    if (!match(TokenType::EQUAL))
      Report(error_msg);

    consume(); // "="
    auto expr = parse_expression();

    if (!match(TokenType::SEMI_COLON))
      Report(error_msg);

    consume(); // ;

    return std::make_unique<VarDecStt>(name, type, std::move(expr));
  }

  std::unique_ptr<Statement> Parser::parse_variable_declaration_data_type() {
    /*
     * Forms:
     *   type x;
     *   type x = something;
     *
     * where something matches the type.
     * Details:
     *   int -> integer value: 1
     *   long -> long integer value: 1L
     *   float -> single-precision float value: 1.00001f
     *   double -> double-precision float value: 1.00000000000001
     *   char -> character value: 'a'
     *   bool -> boolean value: true/false
     */

    std::string type;
    std::string name;

    const std::string error_msg = "Incorrect variable declaration, use:\n"
                                  "\"type x;\"\n"
                                  "\"type x = something;\" where something matches the type.\n"
                                  "Details:\n"
                                  "  int -> integer value: 1\n"
                                  "  long -> long integer value: 1L\n"
                                  "  float -> single-precision float value: 1.00001f\n"
                                  "  double -> double-precision float value: 1.00000000000001\n"
                                  "  char -> character value: 'a'\n"
                                  "  bool -> boolean value: true/false\n";

    type = consume().form;

    if (!match(TokenType::IDENTIFIER))
      Report(error_msg);

    name = consume().form;

    if (match(TokenType::SEMI_COLON)) {
      consume(); // ;
      return std::make_unique<VarDecStt>(name, type, nullptr);
    }

    if (!match(TokenType::EQUAL))
      Report(error_msg);

    consume(); // =

    auto expr = parse_expression();

    if (!match(TokenType::SEMI_COLON))
      Report(error_msg);

    consume(); // ;
    return std::make_unique<VarDecStt>(name, type, std::move(expr));
  }

  std::unique_ptr<Statement> Parser::parse_keyword() {
    const Token identifier = peek();

    // available keywords
    if (identifier.form == "return")
      return parse_return();
    else if (identifier.form == "fn")
      return parse_function();
    else if (identifier.form == "let")
      return parse_variable_declaration_let();
    else
      Report("Undefined keyword");

    return nullptr;
  }

  std::unique_ptr<Statement> Parser::parse_identifier() {
    /* Forms:
     *   function call: name(args);
     *   assignment statement: name = value;
     */

    // assignment/function call are expressions and statements at the same time
    auto expr = parse_expression();

    if (!match(TokenType::SEMI_COLON))
      Report("Expected \";\" after function call statement");

    consume(); // ;

    return std::make_unique<ExprStt>(std::move(expr));
  }

  std::unique_ptr<Statement> Parser::parse_statement() {
    switch (peek().type) {
      case TokenType::KEYWORD:
        return parse_keyword();

      case TokenType::DATA_TYPE:
        return parse_variable_declaration_data_type();

      case TokenType::IDENTIFIER:
        return parse_identifier();

      default:
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
