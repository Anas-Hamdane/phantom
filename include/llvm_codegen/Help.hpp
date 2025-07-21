#ifndef PHANTOM_LLVM_CODEGEN_HELP_HPP
#define PHANTOM_LLVM_CODEGEN_HELP_HPP

#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Type.h"
#include "llvm_codegen/data/VarInfo.hpp"
#include <Logger.hpp>

namespace phantom {
  namespace llvm_codegen {
    namespace Help {
      std::string llvm_to_string(llvm::Type* type, const Logger& logger);
      llvm::Type* string_to_llvm(std::string type, std::shared_ptr<llvm::LLVMContext> context, const Logger& logger);
      TypeInfo::Kind llvm_to_kind(llvm::Type* type, const Logger& logger);
      llvm::Value* default_return(llvm::Type* type, std::shared_ptr<llvm::IRBuilder<>> builder);
    };
  } // namespace llvm_codegen
} // namespace phantom

#endif // !PHANTOM_LLVM_CODEGEN_HELP_HPP
