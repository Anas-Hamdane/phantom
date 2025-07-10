#ifndef PHANTOM_EXPRESSIONS_HPP
#define PHANTOM_EXPRESSIONS_HPP

#include <LLVMCodeGen/ExpressionInfo.hpp>
#include <Lexer/Token.hpp>
#include <global.hpp>

namespace phantom {
  class Visitor;
  class Expression {
public:
    virtual ~Expression() = default;
    virtual ExpressionInfo rvalue(Visitor* visitor) = 0;
    virtual ExpressionInfo lvalue(Visitor* visitor) = 0;
  };

  class DataTypeExpr : public Expression {
public:
    std::string form;
    std::unique_ptr<Expression> value;

    explicit DataTypeExpr(std::unique_ptr<Expression> value, std::string form);
    ExpressionInfo rvalue(Visitor* visitor) override;
    ExpressionInfo lvalue(Visitor* visitor) override;
  };

  class IntLitExpr : public Expression {
public:
    std::string form;
    long long value; // always 8 bytes

    explicit IntLitExpr(std::string form);
    ExpressionInfo rvalue(Visitor* visitor) override;
    ExpressionInfo lvalue(Visitor* visitor) override;
  };

  class FloatLitExpr : public Expression {
public:
    std::string form;
    long double value; // largest possible

    explicit FloatLitExpr(std::string form);
    ExpressionInfo rvalue(Visitor* visitor) override;
    ExpressionInfo lvalue(Visitor* visitor) override;
  };

  class CharLitExpr : public Expression {
public:
    char value;

    explicit CharLitExpr(char value);
    ExpressionInfo rvalue(Visitor* visitor) override;
    ExpressionInfo lvalue(Visitor* visitor) override;
  };

  class BoolLitExpr : public Expression {
public:
    std::string form;
    bool value;

    explicit BoolLitExpr(std::string form);
    ExpressionInfo rvalue(Visitor* visitor) override;
    ExpressionInfo lvalue(Visitor* visitor) override;
  };

  class StrLitExpr : public Expression {
public:
    std::string value;

    explicit StrLitExpr(std::string value);
    ExpressionInfo rvalue(Visitor* visitor) override;
    ExpressionInfo lvalue(Visitor* visitor) override;
  };

  class IdentifierExpr : public Expression {
public:
    std::string name;

    explicit IdentifierExpr(std::string name);
    ExpressionInfo rvalue(Visitor* visitor) override;
    ExpressionInfo lvalue(Visitor* visitor) override;
  };

  class BinOpExpr : public Expression {
public:
    std::unique_ptr<Expression> left;
    std::unique_ptr<Expression> right;
    TokenType op;

    explicit  BinOpExpr(std::unique_ptr<Expression> left, TokenType op, std::unique_ptr<Expression> right);
    ExpressionInfo rvalue(Visitor* visitor) override;
    ExpressionInfo lvalue(Visitor* visitor) override;
  };

  class RefExpr : public Expression {
public:
    std::unique_ptr<IdentifierExpr> ide;

    explicit RefExpr(std::unique_ptr<IdentifierExpr> ide);
    ExpressionInfo rvalue(Visitor* visitor) override;
    ExpressionInfo lvalue(Visitor* visitor) override;
  };

  class DeRefExpr : public Expression {
public:
    std::unique_ptr<Expression> ptr_expr;

    explicit DeRefExpr(std::unique_ptr<Expression> ptr_expr);
    ExpressionInfo rvalue(Visitor* visitor) override;
    ExpressionInfo lvalue(Visitor* visitor) override;
  };

  class FnCallExpr : public Expression {
public:
    std::string name;
    std::vector<std::unique_ptr<Expression>> args;

    explicit FnCallExpr(std::string name, std::vector<std::unique_ptr<Expression>> args);
    ExpressionInfo rvalue(Visitor* visitor) override;
    ExpressionInfo lvalue(Visitor* visitor) override;
  };
} // namespace phantom

#endif // !PHANTOM_EXPRESSIONS_HPP
