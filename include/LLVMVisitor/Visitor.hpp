#ifndef PHANTOM_LLVM_VISITOR_HPP
#define PHANTOM_LLVM_VISITOR_HPP

#include <llvm/IR/Verifier.h>

#include <Parser/Expressions.hpp>
#include <Parser/Statements.hpp>
#include <Data/Variable.hpp>

namespace phantom {
  class Visitor {
    std::unique_ptr<llvm::LLVMContext> context;
    std::unique_ptr<llvm::IRBuilder<>> builder;
    std::unique_ptr<llvm::Module> module;

    std::unordered_map<std::string, Variable> named_values;

public:
    Visitor(const std::string& module_name);

    void print_representation() const;

    // helper function for casting
    llvm::Value* cast(llvm::Value* src, llvm::Type* dst);

    // visit methods
    llvm::Value* visit(IntLitExpr* expr);
    llvm::Value* visit(FloatLitExpr* expr);
    llvm::Value* visit(CharLitExpr* expr);
    llvm::Value* visit(BoolLitExpr* expr);
    llvm::Value* visit(StrLitExpr* expr);

    llvm::Value* visit(IdentifierExpr* expr);
    llvm::Value* visit(BinOpExpr* expr);
    llvm::Value* visit(AddrExpr* expr);
    llvm::Value* visit(FnCallExpr* expr);

    llvm::Value* visit(ReturnStt* stt);
    llvm::Value* visit(ExprStt* stt);

    llvm::Value* visit(VarDecStt* stt);
    llvm::Value* global_var_dec(VarDecStt* stt);
    llvm::Value* local_var_dec(VarDecStt* stt);

    llvm::Function* visit(FnDecStt* stt);
    llvm::Function* visit(FnDefStt* stt);

    llvm::Type* get_llvm_type(std::string type) const {
      if (type == "void")
        return llvm::Type::getVoidTy(*(context));
      else if (type == "bool")
        return llvm::Type::getInt1Ty(*(context));
      else if (type == "char" || type == "byte")
        return llvm::Type::getInt8Ty(*(context));
      else if (type == "short")
        return llvm::Type::getIntNTy(*(context), 16);
      else if (type == "int")
        return llvm::Type::getInt32Ty(*(context));
      else if (type == "long")
        return llvm::Type::getInt64Ty(*(context));
      else if (type == "float")
        return llvm::Type::getFloatTy(*(context));
      else if (type == "double")
        return llvm::Type::getDoubleTy(*(context));
      else if (type == "quad")
        return llvm::Type::getFP128Ty(*context);
      else
        return nullptr;
    }

    std::string get_string_type(llvm::Type* type) const {
      if (type->isIntegerTy(1))
        return "bool";
      else if (type->isIntegerTy(8))
        return "byte";
      else if (type->isIntegerTy(16))
        return "short";
      else if (type->isIntegerTy(32))
        return "int";
      else if (type->isIntegerTy(64))
        return "long";
      else if (type->isFP128Ty())
        return "quad";
      else if (type->isDoubleTy())
        return "double";
      else if (type->isFloatTy())
        return "float";
      else if (type->isVoidTy())
        return "void";
      else
        return nullptr;
    }
  };
} // namespace phantom

#endif // !PHANTOM_LLVM_VISITOR_HPP
