#include <Parser/Statements.hpp>
#include <LLVMVisitor/Visitor.hpp>

namespace phantom {
  Parameter::Parameter(std::string name, std::string type)
      : name(name), type(type) {}

  Statement::~Statement() = default;

  ReturnStt::ReturnStt(std::unique_ptr<Expression> expr)
      : expr(std::move(expr)) {}
  llvm::Value* ReturnStt::accept(Visitor* visitor) { return visitor->visit(this); }

  ExprStt::ExprStt(std::unique_ptr<Expression> expr) : expr(std::move(expr)) {}
  llvm::Value* ExprStt::accept(Visitor* visitor) { return visitor->visit(this); }

  VarDecStt::VarDecStt(std::string name, std::string type, std::unique_ptr<Expression> initializer)
    : name(name), type(type), initializer(std::move(initializer)) {}
  llvm::Value* VarDecStt::accept(Visitor* visitor) { return visitor->visit(this); }

  FnDefStt::FnDefStt(std::unique_ptr<FnDecStt> declaration,
                   std::vector<std::unique_ptr<Statement>> body)
    : declaration(std::move(declaration)), body(std::move(body)) {}
  llvm::Function* FnDefStt::accept(Visitor* visitor) { return visitor->visit(this); }

  FnDecStt::FnDecStt(std::string name, std::string type,
                     std::vector<Parameter> params)
      : name(name), type(type), params(std::move(params)) {}
  llvm::Function* FnDecStt::accept(Visitor* visitor) { return visitor->visit(this); }
} // namespace phantom
