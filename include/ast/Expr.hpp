#ifndef PHANTOM_EXPR_HPP
#define PHANTOM_EXPR_HPP

#include <Token.hpp>
#include <common.hpp>
#include <memory>

namespace phantom {
  class Expr {
public:
    virtual ~Expr() = default;
    // virtual ExprInfo rvalue(Visitor* visitor) = 0;
    // virtual ExprInfo lvalue(Visitor* visitor) = 0;
  };

  class DataTypeExpr : public Expr {
public:
    const std::string type;
    std::unique_ptr<Expr> length;
    std::unique_ptr<Expr> value;

    explicit DataTypeExpr(const std::string& type, std::unique_ptr<Expr> value,
                          std::unique_ptr<Expr> length = nullptr)
        : type(type), value(std::move(value)), length(std::move(length)) {}

    // ExprInfo rvalue(Visitor* visitor) override;
    // ExprInfo lvalue(Visitor* visitor) override;
  };

  class IntLitExpr : public Expr {
public:
    const std::string form;
    uint64_t value;

    explicit IntLitExpr(const std::string& form) : form(form), value(std::stol(form)) {}

    // ExprInfo rvalue(Visitor* visitor) override;
    // ExprInfo lvalue(Visitor* visitor) override;
  };

  class FloatLitExpr : public Expr {
public:
    const std::string form;
    long double value;

    explicit FloatLitExpr(const std::string& form) : form(form), value(std::stold(form)) {}

    // ExprInfo rvalue(Visitor* visitor) override;
    // ExprInfo lvalue(Visitor* visitor) override;
  };

  class CharLitExpr : public Expr {
public:
    char value;

    explicit CharLitExpr(char value) : value(value) {}

    // ExprInfo rvalue(Visitor* visitor) override;
    // ExprInfo lvalue(Visitor* visitor) override;
  };

  class BoolLitExpr : public Expr {
public:
    const std::string form;
    bool value;

    explicit BoolLitExpr(const std::string& form) : form(form), value(form == "true") {}

    // ExprInfo rvalue(Visitor* visitor) override;
    // ExprInfo lvalue(Visitor* visitor) override;
  };

  class StrLitExpr : public Expr {
public:
    const std::string value;

    explicit StrLitExpr(const std::string& value) : value(value) {}

    // ExprInfo rvalue(Visitor* visitor) override;
    // ExprInfo lvalue(Visitor* visitor) override;
  };

  class ArrLitExpr : public Expr {
public:
    std::vector<std::unique_ptr<Expr>> elements;

    explicit ArrLitExpr(std::vector<std::unique_ptr<Expr>> elements)
        : elements(std::move(elements)) {}

    // ExprInfo rvalue(Visitor* visitor) override;
    // ExprInfo lvalue(Visitor* visitor) override;
  };

  class IdeExpr : public Expr {
public:
    const std::string name;

    explicit IdeExpr(const std::string& name) : name(name) {}

    // ExprInfo rvalue(Visitor* visitor) override;
    // ExprInfo lvalue(Visitor* visitor) override;
  };

  class BinOpExpr : public Expr {
public:
    std::unique_ptr<Expr> left;
    std::unique_ptr<Expr> right;
    const Token::Kind op;

    explicit BinOpExpr(std::unique_ptr<Expr> left, const Token::Kind& op, std::unique_ptr<Expr> right)
        : left(std::move(left)), op(op), right(std::move(right)) {}

    // ExprInfo rvalue(Visitor* visitor) override;
    // ExprInfo lvalue(Visitor* visitor) override;
  };

  class VarDecExpr : public Expr {
    const std::string name;
    std::unique_ptr<Expr> expr;

    explicit VarDecExpr(const std::string& name, std::unique_ptr<Expr> expr)
        : name(name), expr(std::move(expr)) {}

    // ExprInfo rvalue(Visitor* visitor) override;
    // ExprInfo lvalue(Visitor* visitor) override;
  };

  class UnaryExpr : public Expr {
    std::unique_ptr<Expr> expr;
    Token::Kind op;

    explicit UnaryExpr(std::unique_ptr<Expr> expr, Token::Kind op)
        : expr(std::move(expr)), op(op) {}
  };

  class FnCallExpr : public Expr {
public:
    const std::string name;
    std::vector<std::unique_ptr<Expr>> args;

    explicit FnCallExpr(const std::string& name, std::vector<std::unique_ptr<Expr>> args)
        : name(name), args(std::move(args)) {}

    // ExprInfo rvalue(Visitor* visitor) override;
    // ExprInfo lvalue(Visitor* visitor) override;
  };
} // namespace phantom

#endif // !PHANTOM_EXPR_HPP
