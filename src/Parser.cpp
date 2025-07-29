#include <Parser.hpp>
#include <string.h>
#include <utils/num.hpp>

namespace phantom {
  // NOTE: the last token *MUST* be of type `Token::Kind::EndOfFile`.
  std::vector<Stmt> Parser::parse() {
    std::vector<Stmt> ast;

    while (true) {
      if (match(Token::Kind::EndOfFile))
        break;

      StmtRef stmt = parse_stmt();
      ast.push_back(stmt);
    }

    return ast;
  }

  Token Parser::consume() {
    if (index >= tokens.size())
      return tokens.back();

    return tokens[index++];
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
    logger.log(Logger::Level::WARNING, "TODO: " + msg, peek().location);
  }

  std::unique_ptr<Stmt> Parser::parse_function() {
    expect(Token::Kind::Fn);
    std::string name = expect(Token::Kind::Identifier);

    expect(Token::Kind::OpenParent);

    std::vector<std::vector<Expr>> params;
    do {
      if (match(Token::Kind::CloseParent))
        break;

      if (match(Token::Kind::Comma))
        consume();

      std::string param_name = expect(Token::Kind::Identifier);
      expect(Token::Kind::Colon);
      std::unique_ptr<Expr> type = parse_type();

      Expr ide{
        .kind = ExprKind::Identifier,
        .data = { .ide = { .name = strdup(param_name.c_str()) } }
      };
      expr_area.add(ide);
      ExprRef ide_ref = expr_area.count - 1;

      Expr expr{
        .kind = ExprKind::VarDecl,
        .data = { .var_decl = { .ide = ide_ref, .value = 0, .type = type } }
      };

      expr_area.add(expr);
      params.push_back(expr_area.count - 1);
    } while (match(Token::Kind::Comma));

    expect(Token::Kind::CloseParent);

    ExprRef type = 0;
    if (match(Token::Kind::RightArrow)) {
      consume();
      type = parse_type();
    }

    else {
      Expr void_type = {
        .kind = ExprKind::DataType,
        .data = { .data_type = { .type = Type::Void, .length = 0 } }
      };

      expr_area.add(void_type);
      type = expr_area.count - 1;
    }

    ExprRef* params_raw = nullptr;
    if (!params.empty()) {
      params_raw = new ExprRef[params.size()];
      std::move(params.begin(), params.end(), params_raw);
    }

    Stmt fn_dec{
      .kind = StmtKind::FnDecl,
      .data = { .fn_decl = {
                    .name = strdup(name.c_str()),
                    .type = type,
                    .params = params_raw,
                    .params_len = params.size() } }
    };
    stmt_area.add(fn_dec);

    StmtRef dec_ref = stmt_area.count - 1;

    if (match(Token::Kind::SemiColon)) {
      consume();
      return dec_ref;
    }

    expect(Token::Kind::OpenCurly);
    std::vector<StmtRef> body;

    while (!match(Token::Kind::CloseCurly))
      body.push_back(parse_stmt());

    expect(Token::Kind::CloseCurly);

    StmtRef* body_raw = nullptr;
    if (!body.empty()) {
      body_raw = new StmtRef[body.size()];
      std::move(body.begin(), body.end(), body_raw);
    }

    Stmt fn_def = {
      .kind = StmtKind::FnDef,
      .data = { .fn_def = { .declaration = dec_ref, .body = body_raw, .body_len = body.size() } }
    };

    stmt_area.add(fn_def);
    return (stmt_area.count - 1);
  }
  StmtRef Parser::parse_return() {
    expect(Token::Kind::Return);
    ExprRef expr = parse_expr();

    expect(Token::Kind::SemiColon);

    Stmt ret{
      .kind = StmtKind::Return,
      .data = { .ret = { .expr = expr } }
    };
    stmt_area.add(ret);
    return (stmt_area.count - 1);
  }
  StmtRef Parser::parse_expmt() {
    ExprRef expr = parse_expr();
    expect(Token::Kind::SemiColon);

    Stmt expmt{
      .kind = StmtKind::Expmt,
      .data = { .expmt = { .expr = expr } }
    };
    stmt_area.add(expmt);
    return (stmt_area.count - 1);
  }
  StmtRef Parser::parse_stmt() {
    switch (peek().kind) {
      case Token::Kind::Fn:
        return parse_function();
      case Token::Kind::Return:
        return parse_return();
      default:
        return parse_expmt();
    }
  }
  ExprRef Parser::parse_expr(const int min_prec) {
    // INFO: Pratt Parser
    ExprRef left = parse_prim();

    while (true) {
      Token op = peek();
      const int prec = Token::precedence(op.kind);

      if (prec <= min_prec) break;
      consume();

      int next_min = Token::right_associative(op.kind) ? prec : (prec + 1);

      ExprRef right = parse_expr(next_min);

      Expr binop{
        .kind = ExprKind::BinOp,
        .data = { .binop = { .left = left, .op = op.kind, .right = right } }
      };
      expr_area.add(binop);

      left = (expr_area.count - 1);
    }

    return left;
  }
  ExprRef Parser::parse_prim() {
    switch (peek().kind) {
      case Token::Kind::Identifier: {
        std::string name = consume().form;

        // function call
        if (match(Token::Kind::OpenParent)) {
          consume();

          std::vector<ExprRef> args;
          do {
            if (match(Token::Kind::CloseParent))
              break;

            if (match(Token::Kind::Comma))
              consume();

            args.push_back(parse_expr());
          } while (match(Token::Kind::Comma));

          expect(Token::Kind::CloseParent);

          ExprRef* args_raw = nullptr;
          if (!args.empty()) {
            args_raw = new ExprRef[args.size()];
            std::move(args.begin(), args.end(), args_raw);
          }

          Expr fn_call{
            .kind = ExprKind::FnCall,
            .data = { .fn_call = { .name = strdup(name.c_str()), .args = args_raw, .len = args.size() } }
          };
          expr_area.add(fn_call);
          return (expr_area.count - 1);
        }

        Expr ide{
          .kind = ExprKind::Identifier,
          .data = { .ide = { .name = strdup(name.c_str()) } }
        };
        expr_area.add(ide);
        return (expr_area.count - 1);
      }
      case Token::Kind::IntLit: {
        std::string form = consume().form;
        std::string log;

        uint64_t value = num::parse_int(form, log);
        if (!log.empty())
          logger.log(Logger::Level::ERROR, log);

        Expr int_lit{
          .kind = ExprKind::IntLit,
          .data = { .int_lit = { .form = strdup(form.c_str()), .value = value } }
        };
        expr_area.add(int_lit);
        return (expr_area.count - 1);
      }
      case Token::Kind::FloatLit: {
        std::string form = consume().form;
        std::string log;

        long double value = num::parse_float(form, log);
        if (!log.empty())
          logger.log(Logger::Level::ERROR, log);

        Expr float_lit{
          .kind = ExprKind::FloatLit,
          .data = { .float_lit = { .form = strdup(form.c_str()), .value = value } }
        };
        expr_area.add(float_lit);
        return (expr_area.count - 1);
      }
      case Token::Kind::Let: {
        consume();
        std::string name = expect(Token::Kind::Identifier);

        ExprRef type = 0;
        ExprRef value = 0;

        if (match(Token::Kind::Colon)) {
          consume();
          type = parse_type();
        }

        if (match(Token::Kind::Eq)) {
          consume();
          value = parse_expr();
        }

        if (value == 0 && type == 0)
          logger.log(Logger::Level::ERROR, "Expected `type` or `value` after variable declaration", peek().location);

        Expr ide{
          .kind = ExprKind::Identifier,
          .data = { .ide = { .name = strdup(name.c_str()) } }
        };
        expr_area.add(ide);
        ExprRef ide_ref = expr_area.count - 1;

        Expr var_decl{
          .kind = ExprKind::VarDecl,
          .data = { .var_decl = { .ide = ide_ref, .value = value, .type = type } }
        };
        expr_area.add(var_decl);
        return (expr_area.count - 1);
      }
      case Token::Kind::DataType:
        return parse_type();
      default:
        todo("Implement support for expressions that starts with `" + Token::kind_to_string(peek().kind) + "`\n");
        return 0;
    }
  }

  ExprRef Parser::parse_type() {
    std::string str_type = expect(Token::Kind::DataType);
    ExprRef length = 0;

    if (match(Token::Kind::OpenBracket)) {
      consume();
      length = parse_expr();

      expect(Token::Kind::CloseBracket);
    }

    Type type = resolve_type(str_type);
    Expr data_type{
      .kind = ExprKind::DataType,
      .data = { .data_type = { .type = type, .length = length } }
    };
    expr_area.add(data_type);
    return (expr_area.count - 1);
  }

  Type Parser::resolve_type(std::string str) {
    if (str == "void")
      return Type::Void;

    if (str == "bool")
      return Type::Bool;

    if (str == "char")
      return Type::Char;

    if (str == "short")
      return Type::Short;

    if (str == "int")
      return Type::Int;

    if (str == "long")
      return Type::Long;

    if (str == "half")
      return Type::Half;

    if (str == "float")
      return Type::Float;

    if (str == "double")
      return Type::Double;

    logger.log(Logger::Level::ERROR, "Unrecognized type: " + str);
    return Type::Int; // default
  }
} // namespace phantom
