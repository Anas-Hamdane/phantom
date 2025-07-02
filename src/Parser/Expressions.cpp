#include <LLVMVisitor/Visitor.hpp>
#include <Parser/Expressions.hpp>

namespace phantom {
  Expression::~Expression() = default;

  IntLitExpr::IntLitExpr(std::string form) : form(form), value(std::stol(form)) {}
  llvm::Value* IntLitExpr::accept(Visitor* visitor) { return visitor->visit(this); }

  FloatLitExpr::FloatLitExpr(std::string form) : form(form), value(std::stod(form)) {}
  llvm::Value* FloatLitExpr::accept(Visitor* visitor) { return visitor->visit(this); }

  ByteLitExpr::ByteLitExpr(int8_t value) : value(value) {}
  llvm::Value* ByteLitExpr::accept(Visitor* visitor) { return visitor->visit(this); }

  BoolLitExpr::BoolLitExpr(std::string form) : form(form), value(form == "true") {}
  llvm::Value* BoolLitExpr::accept(Visitor* visitor) { return visitor->visit(this); }

  StrLitExpr::StrLitExpr(std::string value) : value(value) {}
  llvm::Value* StrLitExpr::accept(Visitor* visitor) { return visitor->visit(this); }

  IDExpr::IDExpr(std::string name) : name(name) {}
  llvm::Value* IDExpr::accept(Visitor* visitor) { return visitor->visit(this); }

  BinOpExpr::BinOpExpr(std::unique_ptr<Expression> left, TokenType op, std::unique_ptr<Expression> right)
      : left(std::move(left)), op(op),
        right(std::move(right)) {}
  llvm::Value* BinOpExpr::accept(Visitor* visitor) { return visitor->visit(this); }

  FnCallExpr::FnCallExpr(std::string name, std::vector<std::unique_ptr<Expression>> args)
      : name(name), args(std::move(args)) {}
  llvm::Value* FnCallExpr::accept(Visitor* visitor) { return visitor->visit(this); }
} // namespace phantom
