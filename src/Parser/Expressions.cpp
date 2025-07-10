#include <LLVMCodeGen/Visitor.hpp>
#include <Parser/Expressions.hpp>

namespace phantom {
  DataTypeExpr::DataTypeExpr(std::unique_ptr<Expression> value, std::string form)
    : value(std::move(value)), form(std::move(form)) {}
  ExpressionInfo DataTypeExpr::rvalue(Visitor* visitor) { return visitor->rvalue(this); }
  ExpressionInfo DataTypeExpr::lvalue(Visitor* visitor) { return visitor->lvalue(this); }

  IntLitExpr::IntLitExpr(std::string form) : form(form), value(std::stol(form)) {}
  ExpressionInfo IntLitExpr::rvalue(Visitor* visitor) { return visitor->rvalue(this); }
  ExpressionInfo IntLitExpr::lvalue(Visitor* visitor) { return visitor->lvalue(this); }

  FloatLitExpr::FloatLitExpr(std::string form) : form(form), value(std::stod(form)) {}
  ExpressionInfo FloatLitExpr::rvalue(Visitor* visitor) { return visitor->rvalue(this); }
  ExpressionInfo FloatLitExpr::lvalue(Visitor* visitor) { return visitor->lvalue(this); }

  CharLitExpr::CharLitExpr(char value) : value(value) {}
  ExpressionInfo CharLitExpr::rvalue(Visitor* visitor) { return visitor->rvalue(this); }
  ExpressionInfo CharLitExpr::lvalue(Visitor* visitor) { return visitor->lvalue(this); }

  BoolLitExpr::BoolLitExpr(std::string form) : form(form), value(form == "true") {}
  ExpressionInfo BoolLitExpr::rvalue(Visitor* visitor) { return visitor->rvalue(this); }
  ExpressionInfo BoolLitExpr::lvalue(Visitor* visitor) { return visitor->lvalue(this); }

  StrLitExpr::StrLitExpr(std::string value) : value(value) {}
  ExpressionInfo StrLitExpr::rvalue(Visitor* visitor) { return visitor->rvalue(this); }
  ExpressionInfo StrLitExpr::lvalue(Visitor* visitor) { return visitor->lvalue(this); }

  IdentifierExpr::IdentifierExpr(std::string name) : name(name) {}
  ExpressionInfo IdentifierExpr::rvalue(Visitor* visitor) { return visitor->rvalue(this); }
  ExpressionInfo IdentifierExpr::lvalue(Visitor* visitor) { return visitor->lvalue(this); }

  BinOpExpr::BinOpExpr(std::unique_ptr<Expression> left, TokenType op, std::unique_ptr<Expression> right)
      : left(std::move(left)), op(op), right(std::move(right)) {}
  ExpressionInfo BinOpExpr::rvalue(Visitor* visitor) { return visitor->rvalue(this); }
  ExpressionInfo BinOpExpr::lvalue(Visitor* visitor) { return visitor->lvalue(this); }

  RefExpr::RefExpr(std::unique_ptr<IdentifierExpr> ide) : ide(std::move(ide)) {}
  ExpressionInfo RefExpr::rvalue(Visitor* visitor) { return visitor->rvalue(this); }
  ExpressionInfo RefExpr::lvalue(Visitor* visitor) { return visitor->lvalue(this); }

  DeRefExpr::DeRefExpr(std::unique_ptr<Expression> ptr_expr) : ptr_expr(std::move(ptr_expr)) {}
  ExpressionInfo DeRefExpr::rvalue(Visitor* visitor) { return visitor->rvalue(this); }
  ExpressionInfo DeRefExpr::lvalue(Visitor* visitor) { return visitor->lvalue(this); }

  FnCallExpr::FnCallExpr(std::string name, std::vector<std::unique_ptr<Expression>> args)
      : name(name), args(std::move(args)) {}
  ExpressionInfo FnCallExpr::rvalue(Visitor* visitor) { return visitor->rvalue(this); }
  ExpressionInfo FnCallExpr::lvalue(Visitor* visitor) { return visitor->lvalue(this); }
} // namespace phantom
