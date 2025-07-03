#include <LLVMVisitor/Visitor.hpp>
#include <Parser/Expressions.hpp>

namespace phantom {
  Expression::~Expression() = default;

  IntLitExpr::IntLitExpr(std::string form) : form(form), value(std::stol(form)) {}
  llvm::Value* IntLitExpr::accept(Visitor* visitor) { return visitor->visit(this); }

  FloatLitExpr::FloatLitExpr(std::string form) : form(form), value(std::stod(form)) {}
  llvm::Value* FloatLitExpr::accept(Visitor* visitor) { return visitor->visit(this); }

  CharLitExpr::CharLitExpr(char value) : value(value) {}
  llvm::Value* CharLitExpr::accept(Visitor* visitor) { return visitor->visit(this); }

  BoolLitExpr::BoolLitExpr(std::string form) : form(form), value(form == "true") {}
  llvm::Value* BoolLitExpr::accept(Visitor* visitor) { return visitor->visit(this); }

  StrLitExpr::StrLitExpr(std::string value) : value(value) {}
  llvm::Value* StrLitExpr::accept(Visitor* visitor) { return visitor->visit(this); }

  IdentifierExpr::IdentifierExpr(std::string name, bool positive) : name(name), positive(positive) {}
  llvm::Value* IdentifierExpr::accept(Visitor* visitor) { return visitor->visit(this); }

  BinOpExpr::BinOpExpr(std::unique_ptr<Expression> left, TokenType op, std::unique_ptr<Expression> right)
      : left(std::move(left)), op(op),
        right(std::move(right)) {}
  llvm::Value* BinOpExpr::accept(Visitor* visitor) { return visitor->visit(this); }

  AddrExpr::AddrExpr(std::string variable) : variable(variable) {}
  llvm::Value* AddrExpr::accept(Visitor* visitor) { return visitor->visit(this); }

  FnCallExpr::FnCallExpr(std::string name, std::vector<std::unique_ptr<Expression>> args)
      : name(name), args(std::move(args)) {}
  llvm::Value* FnCallExpr::accept(Visitor* visitor) { return visitor->visit(this); }
} // namespace phantom
