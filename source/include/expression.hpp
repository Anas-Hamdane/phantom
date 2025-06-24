#ifndef PHANTOM_EXPRESSION_HPP
#define PHANTOM_EXPRESSION_HPP

#include "token.hpp"

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
    IntegerLiteralExpression(long value);
  };

  class IdentifierExpression : public Expression {
    std::string name;

public:
    IdentifierExpression(std::string name);
  };

  class BinaryOpExpression : public Expression {
    std::unique_ptr<Expression> left;
    std::unique_ptr<Expression> right;
    TokenType op;

public:
    BinaryOpExpression(std::unique_ptr<Expression> left, TokenType op, std::unique_ptr<Expression> right);
  };
} // namespace phantom

#endif // !PHANTOM_STATEMENTS_HPP
