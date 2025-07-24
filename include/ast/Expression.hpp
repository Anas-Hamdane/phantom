#ifndef PHANTOM_EXPRESSION_HPP
#define PHANTOM_EXPRESSION_HPP

#include <Token.hpp>
#include <memory>
#include <common.hpp>

namespace phantom {
  class Expression {
public:
    virtual ~Expression() = default;
    virtual ExprInfo rvalue(Visitor* visitor) = 0;
    virtual ExprInfo lvalue(Visitor* visitor) = 0;
  };

  class DataTypeExpr : public Expression {
public:
    const std::string type;
    std::unique_ptr<Expression> value;

    explicit DataTypeExpr(const std::string& type, std::unique_ptr<Expression> value)
        : type(type), value(std::move(value)) {}

    ExprInfo rvalue(Visitor* visitor) override;
    ExprInfo lvalue(Visitor* visitor) override;
  };

  class ArrTypeExpr : public Expression {
public:
    const std::string type;
    std::unique_ptr<Expression> length;
    std::unique_ptr<Expression> value;

    explicit ArrTypeExpr(const std::string& type, std::unique_ptr<Expression> length, std::unique_ptr<Expression> value)
        : type(type), length(std::move(length)), value(std::move(value)) {}

    ExprInfo rvalue(Visitor* visitor) override;
    ExprInfo lvalue(Visitor* visitor) override;
  };

  class IntLitExpr : public Expression {
public:
    const std::string form;
    long long value; // always 8 bytes

    explicit IntLitExpr(const std::string& form) : form(form), value(std::stol(form)) {}

    ExprInfo rvalue(Visitor* visitor) override;
    ExprInfo lvalue(Visitor* visitor) override;
  };

  class FloatLitExpr : public Expression {
public:
    const std::string form;
    long double value; // largest possible

    explicit FloatLitExpr(const std::string& form) : form(form), value(std::stold(form)) {}

    ExprInfo rvalue(Visitor* visitor) override;
    ExprInfo lvalue(Visitor* visitor) override;
  };

  class CharLitExpr : public Expression {
public:
    char value;

    explicit CharLitExpr(char value) : value(value) {}

    ExprInfo rvalue(Visitor* visitor) override;
    ExprInfo lvalue(Visitor* visitor) override;
  };

  class BoolLitExpr : public Expression {
public:
    const std::string form;
    bool value;

    explicit BoolLitExpr(const std::string& form) : form(form), value(form == "true") {}

    ExprInfo rvalue(Visitor* visitor) override;
    ExprInfo lvalue(Visitor* visitor) override;
  };

  class StrLitExpr : public Expression {
public:
    const std::string value;

    explicit StrLitExpr(const std::string& value) : value(value) {}

    ExprInfo rvalue(Visitor* visitor) override;
    ExprInfo lvalue(Visitor* visitor) override;
  };

  class ArrLitExpr : public Expression {
public:
    std::vector<std::unique_ptr<Expression>> elements;

    explicit ArrLitExpr(std::vector<std::unique_ptr<Expression>> elements)
        : elements(std::move(elements)) {}

    ExprInfo rvalue(Visitor* visitor) override;
    ExprInfo lvalue(Visitor* visitor) override;
  };

  class IdeExpr : public Expression {
public:
    const std::string name;

    explicit IdeExpr(const std::string& name) : name(name) {}

    ExprInfo rvalue(Visitor* visitor) override;
    ExprInfo lvalue(Visitor* visitor) override;
  };

  class BinOpExpr : public Expression {
public:
    std::unique_ptr<Expression> left;
    std::unique_ptr<Expression> right;
    const Token::Kind op;

    explicit BinOpExpr(std::unique_ptr<Expression> left, const Token::Kind& op, std::unique_ptr<Expression> right)
        : left(std::move(left)), op(op), right(std::move(right)) {}

    ExprInfo rvalue(Visitor* visitor) override;
    ExprInfo lvalue(Visitor* visitor) override;
  };

  class RefExpr : public Expression {
public:
    std::unique_ptr<IdeExpr> ide;

    explicit RefExpr(std::unique_ptr<IdeExpr> ide) : ide(std::move(ide)) {}

    ExprInfo rvalue(Visitor* visitor) override;
    ExprInfo lvalue(Visitor* visitor) override;
  };

  class DeRefExpr : public Expression {
public:
    std::unique_ptr<Expression> ptr_expr;

    explicit DeRefExpr(std::unique_ptr<Expression> ptr_expr) : ptr_expr(std::move(ptr_expr)) {}

    ExprInfo rvalue(Visitor* visitor) override;
    ExprInfo lvalue(Visitor* visitor) override;
  };

  class FnCallExpr : public Expression {
public:
    const std::string name;
    std::vector<std::unique_ptr<Expression>> args;

    explicit FnCallExpr(const std::string& name, std::vector<std::unique_ptr<Expression>> args)
        : name(name), args(std::move(args)) {}

    ExprInfo rvalue(Visitor* visitor) override;
    ExprInfo lvalue(Visitor* visitor) override;
  };
} // namespace phantom

#endif // !PHANTOM_EXPRESSION_HPP
