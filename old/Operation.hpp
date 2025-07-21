#ifndef PHANTOM_OPERATION_HPP
#define PHANTOM_OPERATION_HPP

#include <Logger.hpp>
#include "data/Data.hpp"
#include "llvm/IR/IRBuilder.h"

namespace phantom {
  namespace llvm_codegen {
    class Operation {
      std::shared_ptr<llvm::IRBuilder<llvm::ConstantFolder, llvm::IRBuilderDefaultInserter>> builder;
      const Logger& logger;

      static int type_precedence(llvm::Type* type);
      void prec_cast(Data& left, Data& right);

  public:
      Operation(std::shared_ptr<llvm::IRBuilder<>>& builder, const Logger& logger)
          : builder(builder), logger(logger) {}

      llvm::Value* cast(llvm::Value* src, llvm::Type* type);
      Data add(Data left, Data right);
      Data sub(Data left, Data right);
      Data mul(Data left, Data right);
      Data div(Data left, Data right);
      Data asgn(Data left, Data right);
    };
  } // namespace llvm_codegen
} // namespace phantom

#endif // !PHANTOM_OPERATION_HPP
