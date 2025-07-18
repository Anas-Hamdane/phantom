#ifndef PHANTOM_OPERATION_HPP
#define PHANTOM_OPERATION_HPP

#include "data/ExprInfo.hpp"
#include "llvm/IR/IRBuilder.h"
#include <Logger.hpp>

namespace phantom {
  namespace llvm_codegen {
    class Operation {
      std::shared_ptr<llvm::IRBuilder<llvm::ConstantFolder, llvm::IRBuilderDefaultInserter>> builder;
      const Logger& logger;

      static int type_precedence(llvm::Type* type);
      llvm::Value* cast(llvm::Value* src, llvm::Type* type);
      void prec_cast(ExprInfo& left, ExprInfo& right);

  public:
      Operation(std::shared_ptr<llvm::IRBuilder<>>& builder, const Logger& logger)
          : builder(builder), logger(logger) {}

      ExprInfo add(ExprInfo left, ExprInfo right);
      ExprInfo sub(ExprInfo left, ExprInfo right);
      ExprInfo mul(ExprInfo left, ExprInfo right);
      ExprInfo div(ExprInfo left, ExprInfo right);
      ExprInfo asgn(ExprInfo left, ExprInfo right);
    };
  } // namespace llvm_codegen
} // namespace phantom

#endif // !PHANTOM_OPERATION_HPP
