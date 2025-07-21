#ifndef PHANTOM_LLVM_VISITOR_HPP
#define PHANTOM_LLVM_VISITOR_HPP

#include "Operation.hpp"
#include <unordered_map>

#include "llvm/Analysis/CGSCCPassManager.h"
#include "llvm/Analysis/LoopAnalysisManager.h"

namespace phantom {
  class DataTypeExpr;
  class ArrTypeExpr;
  class IntLitExpr;
  class FloatLitExpr;
  class CharLitExpr;
  class StrLitExpr;
  class ArrLitExpr;
  class IdeExpr;
  class RefExpr;
  class DeRefExpr;
  class ExprStt;
  class ReturnStt;
  class FnCallExpr;
  class BoolLitExpr;
  class BinOpExpr;
  class VarDecExpr;
  class ArrDecExpr;
  class FnDefStt;
  class FnDecStt;
  namespace llvm_codegen {
    class Visitor {
      bool inside_function = false;
      std::shared_ptr<llvm::LLVMContext> context;
      std::shared_ptr<llvm::IRBuilder<>> builder;
      std::shared_ptr<llvm::Module> module;

      std::unordered_map<std::string, Data> symbol_table;
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
              const Logger& logger)
          : context(context), builder(builder), module(module), logger(logger),
            operation(std::make_unique<Operation>(builder, logger)) {}

      void set_optimizations(std::shared_ptr<llvm::FunctionPassManager> FPM = nullptr,
                             std::shared_ptr<llvm::FunctionAnalysisManager> FAM = nullptr,
                             std::shared_ptr<llvm::LoopAnalysisManager> LAM = nullptr,
                             std::shared_ptr<llvm::ModuleAnalysisManager> MAM = nullptr,
                             std::shared_ptr<llvm::CGSCCAnalysisManager> CGAM = nullptr);

      Data rvalue(DataTypeExpr* expr);
      Data lvalue(DataTypeExpr* expr);

      Data rvalue(ArrTypeExpr* expr);
      Data lvalue(ArrTypeExpr* expr);

      Data rvalue(IntLitExpr* expr);
      Data lvalue(IntLitExpr* expr);

      Data rvalue(FloatLitExpr* expr);
      Data lvalue(FloatLitExpr* expr);

      Data rvalue(CharLitExpr* expr);
      Data lvalue(CharLitExpr* expr);

      Data rvalue(BoolLitExpr* expr);
      Data lvalue(BoolLitExpr* expr);

      Data rvalue(StrLitExpr* expr);
      Data lvalue(StrLitExpr* expr);

      Data rvalue(ArrLitExpr* expr);
      Data lvalue(ArrLitExpr* expr);

      Data rvalue(IdeExpr* expr);
      Data lvalue(IdeExpr* expr);

      Data rvalue(BinOpExpr* expr);
      Data lvalue(BinOpExpr* expr);

      Data rvalue(VarDecExpr* expr);
      Data lvalue(VarDecExpr* expr);
      Data create_global_variable(VarDecExpr* stt);
      Data create_local_variable(VarDecExpr* stt);

      Data rvalue(ArrDecExpr* expr);
      Data lvalue(ArrDecExpr* expr);
      Data create_global_array(ArrDecExpr* stt);
      Data create_local_array(ArrDecExpr* stt);

      Data rvalue(RefExpr* expr);
      Data lvalue(RefExpr* expr);

      Data rvalue(DeRefExpr* expr);
      Data lvalue(DeRefExpr* expr);

      Data rvalue(FnCallExpr* expr);
      Data lvalue(FnCallExpr* expr);

      Data visit(ReturnStt* stt);

      Data visit(ExprStt* stt);

      Data visit(FnDecStt* stt);

      Data visit(FnDefStt* stt);
    };
  } // namespace llvm_codegen
} // namespace phantom

#endif // !PHANTOM_LLVM_VISITOR_HPP
