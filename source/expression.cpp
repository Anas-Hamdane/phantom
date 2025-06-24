#include "include/expression.hpp"

namespace phantom {
  Expression::~Expression() = default;

  IntegerLiteralExpression::IntegerLiteralExpression(long value) : value(value) {}

  IdentifierExpression::IdentifierExpression(std::string name) : name(name) {}

  BinaryOpExpression::BinaryOpExpression(std::unique_ptr<Expression> left, TokenType op,
                                         std::unique_ptr<Expression> right)
      : left(std::move(left)), op(op),
        right(std::move(right)) {}
} // namespace phantom
