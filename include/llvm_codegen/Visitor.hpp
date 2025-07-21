#ifndef PHANTOM_LLVM_VISITOR_HPP
#define PHANTOM_LLVM_VISITOR_HPP

#include "Operation.hpp"
#include <unordered_map>

#include "llvm/Analysis/CGSCCPassManager.h"
#include "llvm/Analysis/LoopAnalysisManager.h"

namespace phantom {
  class Expr;

  class TypeExpr;

  class IntLitExpr;
  class FloatLitExpr;
  class BoolLitExpr;
  class CharLitExpr;
  class StrLitExpr;
  class ArrLitExpr;

  class IdeExpr;
  class RefExpr;
  class DeRefExpr;

  class FnCallExpr;
  class BinOpExpr;
  class VarDecExpr;

  class ExprStt;
  class ReturnStt;
  class FnDefStt;
  class FnDecStt;
  namespace llvm_codegen {
    class Visitor {
  public:
      enum class GenMode {
        Address,
        Load
      };
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

      llvm::Value* make_global_variable(VarDecExpr* expr);
      llvm::Value* make_local_variable(VarDecExpr* expr);

      llvm::Value* make_global_array(std::vector<std::unique_ptr<Expr>> elements);
      llvm::Value* make_local_array(std::vector<std::unique_ptr<Expr>> elements);

      llvm::Value* gen(TypeExpr* expr, GenMode mode = GenMode::Load);

      llvm::Value* gen(IntLitExpr* expr, GenMode mode = GenMode::Load);
      llvm::Value* gen(FloatLitExpr* expr, GenMode mode = GenMode::Load);
      llvm::Value* gen(CharLitExpr* expr, GenMode mode = GenMode::Load);
      llvm::Value* gen(BoolLitExpr* expr, GenMode mode = GenMode::Load);
      llvm::Value* gen(StrLitExpr* expr, GenMode mode = GenMode::Load);
      llvm::Value* gen(ArrLitExpr* expr, GenMode mode = GenMode::Load);

      llvm::Value* gen(IdeExpr* expr, GenMode mode = GenMode::Load);
      llvm::Value* gen(RefExpr* expr, GenMode mode = GenMode::Load);
      llvm::Value* gen(DeRefExpr* expr, GenMode mode = GenMode::Load);

      llvm::Value* gen(BinOpExpr* expr, GenMode mode = GenMode::Load);
      llvm::Value* gen(VarDecExpr* expr, GenMode mode = GenMode::Load);
      llvm::Value* gen(FnCallExpr* expr, GenMode mode = GenMode::Load);

      // llvm::Value* create_global_variable(VarDecExpr* stt);
      // llvm::Value* create_local_variable(VarDecExpr* stt);

      llvm::Value* gen(ReturnStt* stt, GenMode mode = GenMode::Load);
      llvm::Value* gen(ExprStt* stt, GenMode mode = GenMode::Load);
      llvm::Value* gen(FnDecStt* stt, GenMode mode = GenMode::Load);
      llvm::Value* gen(FnDefStt* stt, GenMode mode = GenMode::Load);

  private:
      bool inside_function = false;
      std::shared_ptr<llvm::LLVMContext> context;
      std::shared_ptr<llvm::IRBuilder<>> builder;
      std::shared_ptr<llvm::Module> module;

      std::unordered_map<std::string, VarInfo> name_table;
      std::unordered_map<llvm::Value*, VarInfo> value_table;
      std::unique_ptr<Operation> operation;

      // optimizations
      std::shared_ptr<llvm::FunctionPassManager> FPM = nullptr;
      std::shared_ptr<llvm::FunctionAnalysisManager> FAM = nullptr;
      std::shared_ptr<llvm::LoopAnalysisManager> LAM = nullptr;
      std::shared_ptr<llvm::ModuleAnalysisManager> MAM = nullptr;
      std::shared_ptr<llvm::CGSCCAnalysisManager> CGAM = nullptr;

      const Logger& logger;
    };
  } // namespace llvm_codegen
} // namespace phantom

#endif // !PHANTOM_LLVM_VISITOR_HPP
