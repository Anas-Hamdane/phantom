#include "ast/Parser.hpp"
#include "ast/Expr.hpp"
#include "utils/num.hpp"

namespace phantom {
  namespace ast {
    std::vector<std::unique_ptr<Stmt>> Parser::parse() {
      std::vector<std::unique_ptr<Stmt>> ast;

      while (true) {
        if (match(Token::Kind::EndOfFile))
          break;

        std::unique_ptr<Stmt> stmt = parse_stmt();
        ast.push_back(std::move(stmt));
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

    std::unique_ptr<Stmt> Parser::parse_function() {
      expect(Token::Kind::Fn);
      auto decl = std::make_unique<FnDecl>();
      decl->name = expect(Token::Kind::Identifier);
      expect(Token::Kind::OpenParent);

      do {
        if (match(Token::Kind::CloseParent))
          break;

        if (match(Token::Kind::Comma))
          consume();

        auto param = std::make_unique<VarDecl>();
        param->name = expect(Token::Kind::Identifier);

        expect(Token::Kind::Colon);
        param->type = parse_type();

        decl->params.push_back(std::move(param));
      } while (match(Token::Kind::Comma));

      expect(Token::Kind::CloseParent);
      if (match(Token::Kind::RightArrow)) {
        consume();
        decl->type = parse_type();
      } else {
        // indicate void return
        decl->type = nullptr;
      }

      if (match(Token::Kind::SemiColon)) {
        consume();
        auto stmt = std::make_unique<Stmt>();
        stmt->emplace<std::unique_ptr<FnDecl>>(std::move(decl));
        return stmt;
      }

      expect(Token::Kind::OpenCurly);
      auto def = std::make_unique<FnDef>();
      def->decl = std::move(decl);

      while (!match(Token::Kind::CloseCurly))
        def->body.push_back(parse_stmt());

      expect(Token::Kind::CloseCurly);

      auto stmt = std::make_unique<Stmt>();
      stmt->emplace<std::unique_ptr<FnDef>>(std::move(def));
      return stmt;
    }
    std::unique_ptr<Stmt> Parser::parse_return() {
      expect(Token::Kind::Return);
      auto ret = std::make_unique<Return>();
      ret->expr = parse_expr();
      expect(Token::Kind::SemiColon);

      auto stmt = std::make_unique<Stmt>();
      stmt->emplace<std::unique_ptr<Return>>(std::move(ret));
      return stmt;
    }
    std::unique_ptr<Stmt> Parser::parse_expmt() {
      auto expmt = std::make_unique<Expmt>();
      expmt->expr = parse_expr();
      expect(Token::Kind::SemiColon);

      auto stmt = std::make_unique<Stmt>();
      stmt->emplace<std::unique_ptr<Expmt>>(std::move(expmt));
      return stmt;
    }
    std::unique_ptr<Stmt> Parser::parse_stmt() {
      switch (peek().kind) {
        case Token::Kind::Fn:
          return parse_function();
        case Token::Kind::Return:
          return parse_return();
        default:
          return parse_expmt();
      }
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

        std::unique_ptr<Expr> right = parse_expr(next_min);

        auto binop = std::make_unique<BinOp>();
        binop->lhs = std::move(left);
        binop->rhs = std::move(right);
        binop->op = op.kind;

        auto expr = std::make_unique<Expr>();
        expr->emplace<std::unique_ptr<BinOp>>(std::move(binop));
        left = std::move(expr);
      }

      return left;
    }
    std::unique_ptr<Expr> Parser::parse_prim() {
      switch (peek().kind) {
        case Token::Kind::Identifier: {
          std::string name = consume().form;

          // function call
          if (match(Token::Kind::OpenParent)) {
            consume();
            auto call = std::make_unique<FnCall>();
            call->name = name;

            do {
              if (match(Token::Kind::CloseParent))
                break;

              if (match(Token::Kind::Comma))
                consume();

              call->args.push_back(parse_expr());
            } while (match(Token::Kind::Comma));

            expect(Token::Kind::CloseParent);

            auto expr = std::make_unique<Expr>();
            expr->emplace<std::unique_ptr<FnCall>>(std::move(call));
            return expr;
          }

          auto ide = std::make_unique<Identifier>();
          ide->name = name;

          auto expr = std::make_unique<Expr>();
          expr->emplace<std::unique_ptr<Identifier>>(std::move(ide));
          return expr;
        }
        case Token::Kind::IntLit: {
          std::string form = consume().form;
          std::string log;

          uint64_t value = utils::parse_int(form, log);
          if (!log.empty())
            logger.log(Logger::Level::ERROR, log);

          auto integer = std::make_unique<IntLit>();
          integer->value = value;

          auto expr = std::make_unique<Expr>();
          expr->emplace<std::unique_ptr<IntLit>>(std::move(integer));
          return expr;
        }
        case Token::Kind::FloatLit: {
          std::string form = consume().form;
          std::string log;

          double value = utils::parse_float(form, log);
          if (!log.empty())
            logger.log(Logger::Level::ERROR, log);

          auto fp = std::make_unique<FloatLit>();
          fp->value = value;

          auto expr = std::make_unique<Expr>();
          expr->emplace<std::unique_ptr<FloatLit>>(std::move(fp));
          return expr;
        }
        case Token::Kind::Let: {
          consume();

          auto decl = std::make_unique<VarDecl>();
          decl->name = expect(Token::Kind::Identifier);

          if (match(Token::Kind::Colon)) {
            consume();
            decl->type = parse_type();
          }

          if (match(Token::Kind::Eq)) {
            consume();
            decl->init = parse_expr();
          }

          if (!decl->type && !decl->init)
            logger.log(Logger::Level::ERROR, "Unrecognized type for variable: " + decl->name + "\n");

          auto expr = std::make_unique<Expr>();
          expr->emplace<std::unique_ptr<VarDecl>>(std::move(decl));
          return expr;
        }
        case Token::Kind::OpenParent: {
          consume(); // (
          std::unique_ptr<Expr> expr = parse_expr();
          expect(Token::Kind::CloseParent);
          return expr;
        }
        default:
          // Implement support for expressions that starts with `" + Token::kind_to_string(peek().kind) + "`\n"
          todo();
          return nullptr;
      }
    }

    std::unique_ptr<Type> Parser::parse_type() {
      std::string type_form = expect(Token::Kind::DataType);

      // for "void" it remains 0
      auto type = std::make_unique<Type>();
      type->bitwidth = 0;

      if (type_form == "void")
        return nullptr;

      if (type_form.at(0) == 'i')
        type->kind = Type::Kind::Int;

      // else if (type_form.at(0) == 'u')
      //   type->kind = Type::Kind::UnsInt;

      else if (type_form.at(0) == 'f')
        type->kind = Type::Kind::FP;

      std::string log;
      type->bitwidth = utils::parse_dec(1, type_form, type_form.length(), log);

      if (!log.empty())
        logger.log(Logger::Level::ERROR, log);

      return type;
    }
  } // namespace ast
} // namespace phantom
