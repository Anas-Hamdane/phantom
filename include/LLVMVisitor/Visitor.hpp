#ifndef PHANTOM_LLVM_VISITOR_HPP
#define PHANTOM_LLVM_VISITOR_HPP

#include <llvm/IR/Verifier.h>

#include <Parser/Expressions.hpp>
#include <Parser/Statements.hpp>

namespace phantom {
  class Visitor {
    std::unique_ptr<llvm::LLVMContext> context;
    std::unique_ptr<llvm::IRBuilder<>> builder;
    std::unique_ptr<llvm::Module> module;

    std::unordered_map<std::string, Variable> named_variables;

public:
    Visitor(const std::string& module_name);

    // helper function for casting
    ExpressionInfo global_var_dec(VarDecStt* stt);
    ExpressionInfo local_var_dec(VarDecStt* stt);
    void print_representation() const;

    ExpressionInfo create_addition(ExpressionInfo left, ExpressionInfo right);
    ExpressionInfo create_substraction(ExpressionInfo left, ExpressionInfo right);
    ExpressionInfo create_multiplication(ExpressionInfo left, ExpressionInfo right);
    ExpressionInfo create_division(ExpressionInfo left, ExpressionInfo right);

    ExpressionInfo assign(IdentifierExpr* left, ExpressionInfo right);
    ExpressionInfo assign(DeRefExpr* left, ExpressionInfo right);
    ExpressionInfo handle_assignment(Expression* left, ExpressionInfo right);

    llvm::Value* cast(llvm::Value* src, llvm::Type* dst, std::string error_msg);

    // visit methods
    ExpressionInfo visit(IntLitExpr* expr);
    ExpressionInfo visit(FloatLitExpr* expr);
    ExpressionInfo visit(CharLitExpr* expr);
    ExpressionInfo visit(BoolLitExpr* expr);
    ExpressionInfo visit(StrLitExpr* expr);

    ExpressionInfo visit(IdentifierExpr* expr);
    ExpressionInfo visit(BinOpExpr* expr);
    ExpressionInfo visit(RefExpr* expr);
    ExpressionInfo visit(DeRefExpr* expr);
    ExpressionInfo visit(FnCallExpr* expr);

    ExpressionInfo visit(ReturnStt* stt);
    ExpressionInfo visit(ExprStt* stt);
    ExpressionInfo visit(VarDecStt* stt);

    ExpressionInfo visit(FnDecStt* stt);
    ExpressionInfo visit(FnDefStt* stt);

    llvm::Type* get_llvm_type(std::string type, bool pointer = false) const {
      if (type == "ptr")
        return llvm::PointerType::get(*context, 0);
      else if (type == "void")
        return llvm::Type::getVoidTy(*context);
      else if (type == "bool")
        return llvm::Type::getInt1Ty(*context);
      else if (type == "char" || type == "byte")
        return llvm::Type::getInt8Ty(*context);
      else if (type == "short")
        return llvm::Type::getInt16Ty(*context);
      else if (type == "int")
        return llvm::Type::getInt32Ty(*context);
      else if (type == "long")
        return llvm::Type::getInt64Ty(*context);
      else if (type == "float")
        return llvm::Type::getFloatTy(*context);
      else if (type == "double")
        return llvm::Type::getDoubleTy(*context);
      else if (type == "quad")
        return llvm::Type::getFP128Ty(*context);
      else
        Report("Unidentified data type: \"" + type + "\"\n");

      return nullptr;
    }

    std::string get_string_type(llvm::Type* type) const {
      if (type->isPointerTy())
        return "ptr";
      else if (type->isIntegerTy(1))
        return "bool";
      else if (type->isIntegerTy(8))
        return "char";
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
        Report("Unexpected llvm type\n");

      return "";
    }
  };
} // namespace phantom

#endif // !PHANTOM_LLVM_VISITOR_HPP
