#ifndef PHANTOM_LLVM_VISITOR_HPP
#define PHANTOM_LLVM_VISITOR_HPP

#include "Operation.hpp"
#include "data/Variable.hpp"
#include <unordered_map>

#include "llvm/Analysis/CGSCCPassManager.h"
#include "llvm/Analysis/LoopAnalysisManager.h"

namespace phantom {
  class DataTypeExpr;
  class IntLitExpr;
  class FloatLitExpr;
  class CharLitExpr;
  class StrLitExpr;
  class IdeExpr;
  class RefExpr;
  class DeRefExpr;
  class ExprStt;
  class ReturnStt;
  class VarDecStt;
  class FnCallExpr;
  class BoolLitExpr;
  class BinOpExpr;
  class FnDefStt;
  class FnDecStt;
  namespace llvm_codegen {
    class Visitor {
      std::shared_ptr<llvm::LLVMContext> context;
      std::shared_ptr<llvm::IRBuilder<>> builder;
      std::shared_ptr<llvm::Module> module;

      std::unordered_map<std::string, Variable> named_variables;
      std::unique_ptr<Operation> operation;

      // optimizations
      std::shared_ptr<llvm::FunctionPassManager> FPM = nullptr;
      std::shared_ptr<llvm::FunctionAnalysisManager> FAM = nullptr;
      std::shared_ptr<llvm::LoopAnalysisManager> LAM = nullptr;
      std::shared_ptr<llvm::ModuleAnalysisManager> MAM = nullptr;
      std::shared_ptr<llvm::CGSCCAnalysisManager> CGAM = nullptr;

      const Logger& logger;

  public:
      Visitor(std::shared_ptr<llvm::LLVMContext> context,
              std::shared_ptr<llvm::IRBuilder<>> builder,
              std::shared_ptr<llvm::Module> module,
              const Logger& logger) : context(context), builder(builder), module(module), logger(logger) {}

      void set_optimizations(std::shared_ptr<llvm::FunctionPassManager> FPM = nullptr,
                             std::shared_ptr<llvm::FunctionAnalysisManager> FAM = nullptr,
                             std::shared_ptr<llvm::LoopAnalysisManager> LAM = nullptr,
                             std::shared_ptr<llvm::ModuleAnalysisManager> MAM = nullptr,
                             std::shared_ptr<llvm::CGSCCAnalysisManager> CGAM = nullptr);

      // helper function for casting
      llvm::Value* cast(llvm::Value* src, llvm::Type* dst, std::string error_msg);

      ExprInfo rvalue(DataTypeExpr* expr);
      ExprInfo lvalue(DataTypeExpr* expr);

      ExprInfo rvalue(IntLitExpr* expr);
      ExprInfo lvalue(IntLitExpr* expr);

      ExprInfo rvalue(FloatLitExpr* expr);
      ExprInfo lvalue(FloatLitExpr* expr);

      ExprInfo rvalue(CharLitExpr* expr);
      ExprInfo lvalue(CharLitExpr* expr);

      ExprInfo rvalue(BoolLitExpr* expr);
      ExprInfo lvalue(BoolLitExpr* expr);

      ExprInfo rvalue(StrLitExpr* expr);
      ExprInfo lvalue(StrLitExpr* expr);

      ExprInfo rvalue(IdeExpr* expr);
      ExprInfo lvalue(IdeExpr* expr);

      ExprInfo rvalue(BinOpExpr* expr);
      ExprInfo lvalue(BinOpExpr* expr);

      ExprInfo rvalue(RefExpr* expr);
      ExprInfo lvalue(RefExpr* expr);

      ExprInfo rvalue(DeRefExpr* expr);
      ExprInfo lvalue(DeRefExpr* expr);

      ExprInfo rvalue(FnCallExpr* expr);
      ExprInfo lvalue(FnCallExpr* expr);

      ExprInfo visit(ReturnStt* stt);

      ExprInfo visit(ExprStt* stt);

      ExprInfo global_var_dec(VarDecStt* stt);
      ExprInfo local_var_dec(VarDecStt* stt);
      ExprInfo visit(VarDecStt* stt);

      ExprInfo visit(FnDecStt* stt);

      ExprInfo visit(FnDefStt* stt);

      llvm::Type* get_llvm_type(std::string type, bool pointer = false) const;

      std::string get_string_type(llvm::Type* type) const;
    };
  } // namespace llvm_codegen
} // namespace phantom

#endif // !PHANTOM_LLVM_VISITOR_HPP
