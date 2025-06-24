#ifndef PHANTOM_EXPRESSION_HPP
#define PHANTOM_EXPRESSION_HPP

#include "../lexer/token.hpp"

#include <memory>
#include <string>

namespace phantom {
  class Expression {
public:
    virtual ~Expression();
  };

  class IntegerLiteralExpression : public Expression {
    long value;

public:
    explicit IntegerLiteralExpression(long value);
  };

  class IdentifierExpression : public Expression {
    std::string name;

public:
    explicit IdentifierExpression(std::string name);
  };

  class BinaryOpExpression : public Expression {
    std::unique_ptr<Expression> left;
    std::unique_ptr<Expression> right;
    TokenType op;

public:
    BinaryOpExpression(std::unique_ptr<Expression> left, TokenType op, std::unique_ptr<Expression> right);
  };
} // namespace phantom

#endif // PHANTOM_EXPRESSION_HPP
