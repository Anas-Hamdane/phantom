#include <Parser.hpp>

namespace phantom {
  // NOTE: the last token *MUST* be of type `Token::Kind::EndOfFile`.
  std::vector<std::unique_ptr<Stmt>> Parser::parse() {
    std::vector<std::unique_ptr<Stmt>> ast;

    // TODO: Uncompleted
    while (true) {
      if (match(Token::Kind::EndOfFile))
        break;

      switch (peek().kind) {
        case Token::Kind::Fn:
          parse_function();
        default:
          todo("TODO: Implement parse for token '" + Token::kind_to_string(peek().kind) + "'");
      }
    }

    return ast;
  }

  Token Parser::consume() {
    if (index >= tokens.size())
      return tokens.back();

    Token token = tokens[index];

    index++;
    return token;
  }
  Token Parser::peek(off_t offset) const {
    if (index + offset >= tokens.size())
      return tokens.back();

    return tokens[index + offset];
  }
  bool Parser::match(Token::Kind kind, off_t offset) const {
    if ((index + offset) >= tokens.size())
      return (kind == tokens.back().kind);

    return (kind == tokens[index + offset].kind);
  }

  std::string Parser::expect(Token::Kind kind) {
    if (match(kind))
      return consume().form;

    logger.log(Logger::Level::ERROR, "Expected token '" + Token::kind_to_string(kind) + "', got '" + Token::kind_to_string(peek().kind) + "'", peek().location);
    return "";
  }
  void Parser::todo(const std::string& msg) {
    logger.log(Logger::Level::WARNING, msg, peek().location);
  }

  std::unique_ptr<Stmt> Parser::parse_function() {
    expect(Token::Kind::Let);

    std::string fn_name = expect(Token::Kind::Identifier);

    expect(Token::Kind::OpenParent);

    std::vector<std::unique_ptr<VarDecExpr>> params;
    do {
      if (match(Token::Kind::CloseParent))
        break;

      if (match(Token::Kind::Comma))
        consume();

      std::string param_name = expect(Token::Kind::Identifier);

      todo("Implement data types parsing");
      consume();
    } while (match(Token::Kind::Comma));

    expect(Token::Kind::DRArrow);
    std::unique_ptr<DataTypeExpr> type = parse_type();

    std::unique_ptr<FnDecStmt> declaration = std::make_unique<FnDecStmt>(fn_name, std::move(type), std::move(params));

    if (match(Token::Kind::SemiColon))
      return std::move(declaration);

    expect(Token::Kind::OpenCurly);

    std::vector<std::unique_ptr<Stmt>> body;
    while (!match(Token::Kind::CloseCurly))
      body.push_back(parse_stmt());

    expect(Token::Kind::CloseCurly);

    return std::make_unique<FnDefStmt>(std::move(declaration), std::move(body));
  }
  std::unique_ptr<Stmt> Parser::parse_return() {
    expect(Token::Kind::Return);

    std::unique_ptr<Expr> expr = parse_expr();
    return std::make_unique<RetStmt>(std::move(expr));
  }
  std::unique_ptr<Stmt> Parser::parse_expmt() {
    // INFO: parse Exprs as Stmts
    std::unique_ptr<Expr> expr = parse_expr();
    expect(Token::Kind::SemiColon);

    return std::make_unique<ExprStmt>(std::move(expr));
  }
  std::unique_ptr<Stmt> Parser::parse_stmt() {
    switch (peek().kind) {
      case Token::Kind::Fn:
        return parse_function();
      case Token::Kind::Return:
        return parse_return();
      default:
        todo("TODO: Implement parse for token '" + Token::kind_to_string(peek().kind) + "'");
    }

    return nullptr;
  }

  std::unique_ptr<Expr> Parser::parse_expr(const int min_prec) {
    // INFO: Pratt Parser
    std::unique_ptr<Expr> left = parse_prim();

    while (true) {
      Token op = peek();
      const int prec = Token::precedence(op.kind);

      if (prec <= min_prec) break;
      consume();

      int next_min = Token::right_associative(op.kind) ? prec : (prec + 1);

      auto right = parse_expr(next_min);
      left = std::make_unique<BinOpExpr>(std::move(left), op.kind, std::move(right));
    }

    return std::move(left);
  }
  std::unique_ptr<Expr> Parser::parse_prim() {
    // TODO: keml hadxi
    if (match(Token::Kind::Identifier))
      return std::make_unique<IdeExpr>(consume().form);

    if (match(Token::Kind::DataType))
      return parse_type();

    return nullptr;
  }

  std::unique_ptr<DataTypeExpr> Parser::parse_type() {
    std::string type = expect(Token::Kind::DataType);
    std::unique_ptr<Expr> length = nullptr;

    if (match(Token::Kind::OpenBracket)) {
      consume();
      length = parse_expr();

      expect(Token::Kind::CloseBracket);
    }

    return std::make_unique<DataTypeExpr>(type, std::move(length));
  }
} // namespace phantom
