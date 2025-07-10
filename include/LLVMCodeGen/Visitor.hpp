#ifndef PHANTOM_LLVM_VISITOR_HPP
#define PHANTOM_LLVM_VISITOR_HPP

#include <llvm/IR/Verifier.h>

#include <LLVMCodeGen/Operation.hpp>
#include <Parser/Expressions.hpp>
#include <Parser/Statements.hpp>

namespace phantom {
  class Visitor {
    std::unique_ptr<llvm::LLVMContext> context;
    std::unique_ptr<llvm::Module> module;

    std::unordered_map<std::string, Variable> named_variables;

    std::shared_ptr<llvm::IRBuilder<>> builder;
    Operation operation;

public:
    Visitor(const std::string& module_name);

    // helper function for casting
    ExpressionInfo global_var_dec(VarDecStt* stt);
    ExpressionInfo local_var_dec(VarDecStt* stt);
    void print_representation() const;
    llvm::Value* cast(llvm::Value* src, llvm::Type* dst, std::string error_msg);

    ExpressionInfo rvalue(DataTypeExpr* expr);
    ExpressionInfo lvalue(DataTypeExpr* expr);

    ExpressionInfo rvalue(IntLitExpr* expr);
    ExpressionInfo lvalue(IntLitExpr* expr);

    ExpressionInfo rvalue(FloatLitExpr* expr);
    ExpressionInfo lvalue(FloatLitExpr* expr);

    ExpressionInfo rvalue(CharLitExpr* expr);
    ExpressionInfo lvalue(CharLitExpr* expr);

    ExpressionInfo rvalue(BoolLitExpr* expr);
    ExpressionInfo lvalue(BoolLitExpr* expr);

    ExpressionInfo rvalue(StrLitExpr* expr);
    ExpressionInfo lvalue(StrLitExpr* expr);

    ExpressionInfo rvalue(IdentifierExpr* expr);
    ExpressionInfo lvalue(IdentifierExpr* expr);

    ExpressionInfo rvalue(BinOpExpr* expr);
    ExpressionInfo lvalue(BinOpExpr* expr);

    ExpressionInfo rvalue(RefExpr* expr);
    ExpressionInfo lvalue(RefExpr* expr);

    ExpressionInfo rvalue(DeRefExpr* expr);
    ExpressionInfo lvalue(DeRefExpr* expr);

    ExpressionInfo rvalue(FnCallExpr* expr);
    ExpressionInfo lvalue(FnCallExpr* expr);

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
      else if (type == "char")
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
