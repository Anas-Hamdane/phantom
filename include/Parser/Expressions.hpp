#ifndef PHANTOM_EXPRESSIONS_HPP
#define PHANTOM_EXPRESSIONS_HPP

#include <Data/ExpressionInfo.hpp>
#include <Lexer/Token.hpp>
#include <global.hpp>

namespace phantom {
  class Visitor;
  class Expression {
public:
    virtual ~Expression();
    virtual ExpressionInfo accept(Visitor* visitor) = 0;
  };

  class IntLitExpr : public Expression {
public:
    std::string form;
    long long value; // always 8 bytes

    explicit IntLitExpr(std::string form);
    ExpressionInfo accept(Visitor* visitor) override;
  };

  class FloatLitExpr : public Expression {
public:
    std::string form;
    long double value; // largest possible

    explicit FloatLitExpr(std::string form);
    ExpressionInfo accept(Visitor* visitor) override;
  };

  class CharLitExpr : public Expression {
public:
    char value;

    explicit CharLitExpr(char value);
    ExpressionInfo accept(Visitor* visitor) override;
  };

  class BoolLitExpr : public Expression {
public:
    std::string form;
    bool value;

    explicit BoolLitExpr(std::string form);
    ExpressionInfo accept(Visitor* visitor) override;
  };

  class StrLitExpr : public Expression {
public:
    std::string value;

    explicit StrLitExpr(std::string value);
    ExpressionInfo accept(Visitor* visitor) override;
  };

  class IdentifierExpr : public Expression {
public:
    std::string name;

    explicit IdentifierExpr(std::string name);
    ExpressionInfo accept(Visitor* visitor) override;
    ExpressionInfo assign(ExpressionInfo right, Visitor* visitor);
  };

  class BinOpExpr : public Expression {
public:
    std::unique_ptr<Expression> left;
    std::unique_ptr<Expression> right;
    TokenType op;

    BinOpExpr(std::unique_ptr<Expression> left, TokenType op, std::unique_ptr<Expression> right);
    ExpressionInfo accept(Visitor* visitor) override;
  };

  class RefExpr : public Expression {
public:
    std::unique_ptr<IdentifierExpr> ide;

    RefExpr(std::unique_ptr<IdentifierExpr> ide);
    ExpressionInfo accept(Visitor* visitor) override;
  };

  class DeRefExpr : public Expression {
public:
    std::unique_ptr<Expression> ptr_expr;

    DeRefExpr(std::unique_ptr<Expression> ptr_expr);
    ExpressionInfo accept(Visitor* visitor) override;
    ExpressionInfo assign(ExpressionInfo right, Visitor* visitor);
  };

  class FnCallExpr : public Expression {
public:
    std::string name;
    std::vector<std::unique_ptr<Expression>> args;

    explicit FnCallExpr(std::string name, std::vector<std::unique_ptr<Expression>> args);
    ExpressionInfo accept(Visitor* visitor) override;
  };
} // namespace phantom

#endif // !PHANTOM_EXPRESSIONS_HPP
