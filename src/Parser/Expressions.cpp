#include <LLVMVisitor/Visitor.hpp>
#include <Parser/Expressions.hpp>

namespace phantom {
  Expression::~Expression() = default;

  IntLitExpr::IntLitExpr(std::string form) : form(form), value(std::stol(form)) {}
  ExpressionInfo IntLitExpr::accept(Visitor* visitor) { return visitor->visit(this); }

  FloatLitExpr::FloatLitExpr(std::string form) : form(form), value(std::stod(form)) {}
  ExpressionInfo FloatLitExpr::accept(Visitor* visitor) { return visitor->visit(this); }

  CharLitExpr::CharLitExpr(char value) : value(value) {}
  ExpressionInfo CharLitExpr::accept(Visitor* visitor) { return visitor->visit(this); }

  BoolLitExpr::BoolLitExpr(std::string form) : form(form), value(form == "true") {}
  ExpressionInfo BoolLitExpr::accept(Visitor* visitor) { return visitor->visit(this); }

  StrLitExpr::StrLitExpr(std::string value) : value(value) {}
  ExpressionInfo StrLitExpr::accept(Visitor* visitor) { return visitor->visit(this); }

  IdentifierExpr::IdentifierExpr(std::string name) : name(name) {}
  ExpressionInfo IdentifierExpr::accept(Visitor* visitor) { return visitor->visit(this); }
  ExpressionInfo IdentifierExpr::assign(ExpressionInfo right, Visitor* visitor) { return visitor->assign(this, right); }

  BinOpExpr::BinOpExpr(std::unique_ptr<Expression> left, TokenType op, std::unique_ptr<Expression> right)
      : left(std::move(left)), op(op),
        right(std::move(right)) {}
  ExpressionInfo BinOpExpr::accept(Visitor* visitor) { return visitor->visit(this); }

  RefExpr::RefExpr(std::unique_ptr<IdentifierExpr> ide) : ide(std::move(ide)) {}
  ExpressionInfo RefExpr::accept(Visitor* visitor) { return visitor->visit(this); }

  DeRefExpr::DeRefExpr(std::unique_ptr<Expression> ptr_expr) : ptr_expr(std::move(ptr_expr)) {}
  ExpressionInfo DeRefExpr::accept(Visitor* visitor) { return visitor->visit(this); }
  ExpressionInfo DeRefExpr::assign(ExpressionInfo right, Visitor* visitor) { return visitor->assign(this, right); }

  FnCallExpr::FnCallExpr(std::string name, std::vector<std::unique_ptr<Expression>> args)
      : name(name), args(std::move(args)) {}
  ExpressionInfo FnCallExpr::accept(Visitor* visitor) { return visitor->visit(this); }
} // namespace phantom
