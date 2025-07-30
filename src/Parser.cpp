#include "ast/Expr.hpp"
#include <Parser.hpp>
#include <string.h>
#include <utils/num.hpp>

namespace phantom {
  // NOTE: the last token *MUST* be of type `Token::Kind::EndOfFile`.
  utils::Vec<Stmt> Parser::parse() {
    utils::Vec<Stmt> ast;
    ast.init();

    while (true) {
      if (match(Token::Kind::EndOfFile))
        break;

      Stmt stmt = parse_stmt();
      ast.push(stmt);
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

  Stmt Parser::parse_function() {
    expect(Token::Kind::Fn);
    std::string name = expect(Token::Kind::Identifier);

    expect(Token::Kind::OpenParent);

    utils::Vec<Param> params;
    params.init();
    do {
      if (match(Token::Kind::CloseParent))
        break;

      if (match(Token::Kind::Comma))
        consume();

      std::string param_name = expect(Token::Kind::Identifier);
      expect(Token::Kind::Colon);
      DataType type = parse_type();

      Identifier* ide = new Identifier;
      ide->name = strdup(param_name.c_str());

      Param param;
      param.ide = ide;
      param.type = new DataType(type);

      params.push(param);
    } while (match(Token::Kind::Comma));

    expect(Token::Kind::CloseParent);

    DataType type;
    if (match(Token::Kind::RightArrow)) {
      consume();
      type = parse_type();
    } else {
      type.type = Type::Void;
      type.length = nullptr;
    }

    FnDecl decl;
    decl.type = new DataType(type);
    decl.name = strdup(name.c_str());
    decl.params = std::move(params);

    if (match(Token::Kind::SemiColon)) {
      consume();
      return Stmt{ .kind = StmtKind::FnDecl, .data = { .fn_decl = decl } };
    }

    expect(Token::Kind::OpenCurly);
    utils::Vec<Stmt> body;
    body.init();

    while (!match(Token::Kind::CloseCurly))
      body.push(parse_stmt());

    expect(Token::Kind::CloseCurly);

    Stmt fn;
    fn.kind = StmtKind::FnDef;
    fn.data.fn_def.declaration = new FnDecl(decl);
    fn.data.fn_def.body = std::move(body);

    return fn;
  }
  Stmt Parser::parse_return() {
    expect(Token::Kind::Return);
    Expr expr = parse_expr();

    expect(Token::Kind::SemiColon);

    Stmt ret;
    ret.kind = StmtKind::Return;
    ret.data.ret.expr = new Expr(expr);

    return ret;
  }
  Stmt Parser::parse_expmt() {
    Expr expr = parse_expr();
    expect(Token::Kind::SemiColon);

    Stmt expmt;
    expmt.kind = StmtKind::Expmt;
    expmt.data.expmt.expr = new Expr(expr);

    return expmt;
  }
  Stmt Parser::parse_stmt() {
    switch (peek().kind) {
      case Token::Kind::Fn:
        return parse_function();
      case Token::Kind::Return:
        return parse_return();
      default:
        return parse_expmt();
    }
  }

  Expr Parser::parse_expr(const int min_prec) {
    // INFO: Pratt Parser
    Expr left = parse_prim();

    while (true) {
      Token op = peek();
      const int prec = Token::precedence(op.kind);

      if (prec <= min_prec) break;
      consume();

      int next_min = Token::right_associative(op.kind) ? prec : (prec + 1);

      Expr right = parse_expr(next_min);

      Expr binop;
      binop.kind = ExprKind::BinOp;
      binop.data.binop.lhs = new Expr(left);
      binop.data.binop.rhs = new Expr(right);
      binop.data.binop.op = op.kind;

      left = std::move(binop);
    }

    return left;
  }
  Expr Parser::parse_prim() {
    switch (peek().kind) {
      case Token::Kind::Identifier: {
        std::string name = consume().form;

        // function call
        if (match(Token::Kind::OpenParent)) {
          consume();

          utils::Vec<Expr> args;
          args.init();
          do {
            if (match(Token::Kind::CloseParent))
              break;

            if (match(Token::Kind::Comma))
              consume();

            args.push(parse_expr());
          } while (match(Token::Kind::Comma));

          expect(Token::Kind::CloseParent);

          Expr fn_call;
          fn_call.kind = ExprKind::FnCall;
          fn_call.data.fn_call.name = strdup(name.c_str());
          fn_call.data.fn_call.args = std::move(args);

          return fn_call;
        }

        Expr ide;
        ide.kind = ExprKind::Identifier;
        ide.data.ide.name = strdup(name.c_str());

        return ide;
      }
      case Token::Kind::IntLit: {
        std::string form = consume().form;
        std::string log;

        uint64_t value = utils::parse_int(form, log);
        if (!log.empty())
          logger.log(Logger::Level::ERROR, log);

        Expr int_lit;
        int_lit.kind = ExprKind::IntLit;
        int_lit.data.int_lit.form = strdup(form.c_str());
        int_lit.data.int_lit.value = value;

        return int_lit;
      }
      case Token::Kind::FloatLit: {
        std::string form = consume().form;
        std::string log;

        double value = utils::parse_float(form, log);
        if (!log.empty())
          logger.log(Logger::Level::ERROR, log);

        Expr float_lit;
        float_lit.kind = ExprKind::FloatLit;
        float_lit.data.float_lit.form = strdup(form.c_str());
        float_lit.data.float_lit.value = value;

        return float_lit;
      }
      case Token::Kind::Let: {
        consume();
        std::string name = expect(Token::Kind::Identifier);

        DataType* type = nullptr;
        Expr* value = nullptr;

        if (match(Token::Kind::Colon)) {
          consume();
          type = new DataType(parse_type());
        }

        if (match(Token::Kind::Eq)) {
          consume();
          value = new Expr(parse_expr());
        }

        if (value == nullptr && type == nullptr)
          logger.log(Logger::Level::ERROR, "Expected `type` or `value` after variable declaration", peek().location);

        Identifier ide;
        ide.name = strdup(name.c_str());

        Expr var_decl;
        var_decl.kind = ExprKind::VarDecl;
        var_decl.data.var_decl.ide = new Identifier(ide);
        var_decl.data.var_decl.value = value;
        var_decl.data.var_decl.type = type;

        return var_decl;
      }
      case Token::Kind::DataType: {
        Expr expr{
          .kind = ExprKind::DataType,
          .data = { .data_type = parse_type() },
        };
        return expr;
      }
      default:
        todo("Implement support for expressions that starts with `" + Token::kind_to_string(peek().kind) + "`\n");
        return Expr();
    }
  }

  DataType Parser::parse_type() {
    std::string str_type = expect(Token::Kind::DataType);
    Expr* length = nullptr;

    if (match(Token::Kind::OpenBracket)) {
      consume();
      length = new Expr(parse_expr());

      expect(Token::Kind::CloseBracket);
    }

    Type type = resolve_type(str_type);

    DataType data_type;
    data_type.type = type;
    data_type.length = length;

    return data_type;
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
