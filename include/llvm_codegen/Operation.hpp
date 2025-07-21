#ifndef PHANTOM_OPERATION_HPP
#define PHANTOM_OPERATION_HPP

#include "llvm/IR/IRBuilder.h"
#include "data/VarInfo.hpp"
#include <Logger.hpp>

namespace phantom {
  namespace llvm_codegen {
    class Operation {
  public:
      Operation(std::shared_ptr<llvm::IRBuilder<>>& builder, const Logger& logger)
          : builder(builder), logger(logger) {}

      llvm::Value** resolve_value(llvm::Value* value);
      llvm::Value** cast(llvm::Value** src, llvm::Type* type);

      llvm::Value* load(llvm::Value** value, llvm::Type* type);
      llvm::Value* store(llvm::Value** left, llvm::Value** ptr);
      llvm::Value* call(llvm::Function* fn, std::vector<llvm::Value**> args);

      llvm::Value* add(llvm::Value* left, llvm::Value* right);
      llvm::Value* sub(llvm::Value* left, llvm::Value* right);
      llvm::Value* mul(llvm::Value* left, llvm::Value* right);
      llvm::Value* div(llvm::Value* left, llvm::Value* right);
      llvm::Value* asgn(llvm::Value* left, llvm::Value* right);

  private:
      std::shared_ptr<llvm::IRBuilder<>> builder;
      const Logger& logger;

      static int type_precedence(llvm::Type* type);
      void prec_cast(llvm::Value*& left, llvm::Value*& right);
    };
  } // namespace llvm_codegen
} // namespace phantom

#endif // !PHANTOM_OPERATION_HPP
