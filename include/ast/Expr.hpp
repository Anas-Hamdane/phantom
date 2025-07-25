#pragma once

#include <Token.hpp>
#include <common.hpp>
#include <memory>

#include "Elm.hpp"

namespace phantom {
  class Expr {
public:
    virtual ~Expr() = default;
    virtual AstElm represent() const = 0;
  };

  class DataTypeExpr : public Expr {
public:
    const std::string type;
    std::unique_ptr<Expr> length;

    explicit DataTypeExpr(const std::string& type, std::unique_ptr<Expr> length = nullptr)
        : type(type), length(std::move(length)) {}

    AstElm represent() const override;
  };

  class IntLitExpr : public Expr {
public:
    const std::string form;
    uint64_t value;

    explicit IntLitExpr(const std::string& form, uint64_t value) : form(form), value(value) {}

    AstElm represent() const override;
  };

  class FloatLitExpr : public Expr {
public:
    const std::string form;
    long double value;

    explicit FloatLitExpr(const std::string& form, long double value) : form(form), value(value) {}

    AstElm represent() const override;
  };

  class CharLitExpr : public Expr {
public:
    char value;

    explicit CharLitExpr(char value) : value(value) {}

    AstElm represent() const override;
  };

  class BoolLitExpr : public Expr {
public:
    const std::string form;
    bool value;

    explicit BoolLitExpr(const std::string& form) : form(form), value(form == "true") {}

    AstElm represent() const override;
  };

  class StrLitExpr : public Expr {
public:
    const std::string value;

    explicit StrLitExpr(const std::string& value) : value(value) {}

    AstElm represent() const override;
  };

  class ArrLitExpr : public Expr {
public:
    std::vector<std::unique_ptr<Expr>> elements;

    explicit ArrLitExpr(std::vector<std::unique_ptr<Expr>> elements)
        : elements(std::move(elements)) {}

    AstElm represent() const override;
  };

  class IdeExpr : public Expr {
public:
    const std::string name;

    explicit IdeExpr(const std::string& name) : name(name) {}

    AstElm represent() const override;
  };

  class BinOpExpr : public Expr {
public:
    std::unique_ptr<Expr> left;
    const Token::Kind op;
    std::unique_ptr<Expr> right;

    explicit BinOpExpr(std::unique_ptr<Expr> left, const Token::Kind& op, std::unique_ptr<Expr> right)
        : left(std::move(left)), op(op), right(std::move(right)) {}

    AstElm represent() const override;
  };

  class VarDecExpr : public Expr {
public:
    const std::string name;
    std::unique_ptr<Expr> value;
    std::unique_ptr<Expr> type;

    explicit VarDecExpr(const std::string& name, std::unique_ptr<Expr> type, std::unique_ptr<Expr> value)
        : name(name), value(std::move(value)), type(std::move(type)) {}

    AstElm represent() const override;
  };

  class UnaryExpr : public Expr {
public:
    std::unique_ptr<Expr> expr;
    Token::Kind op;

    explicit UnaryExpr(std::unique_ptr<Expr> expr, Token::Kind op)
        : expr(std::move(expr)), op(op) {}

    AstElm represent() const override;
  };

  class FnCallExpr : public Expr {
public:
    const std::string name;
    std::vector<std::unique_ptr<Expr>> args;

    explicit FnCallExpr(const std::string& name, std::vector<std::unique_ptr<Expr>> args)
        : name(name), args(std::move(args)) {}

    AstElm represent() const override;
  };
} // namespace phantom
