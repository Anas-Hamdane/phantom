#ifndef PHANTOM_OPERATION_HPP
#define PHANTOM_OPERATION_HPP

#include <LLVMCodeGen/ExpressionInfo.hpp>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Type.h>

namespace phantom {
  class Operation {
    std::shared_ptr<llvm::IRBuilder<>> builder;

    static int type_precedence(llvm::Type* type);
    llvm::Value* cast(llvm::Value* src, llvm::Type* type);
    void prec_cast(ExpressionInfo& left, ExpressionInfo& right);

public:
    Operation(std::shared_ptr<llvm::IRBuilder<>> builder);

    ExpressionInfo add(ExpressionInfo left, ExpressionInfo right);
    ExpressionInfo sub(ExpressionInfo left, ExpressionInfo right);
    ExpressionInfo mul(ExpressionInfo left, ExpressionInfo right);
    ExpressionInfo div(ExpressionInfo left, ExpressionInfo right);
    ExpressionInfo asgn(ExpressionInfo left, ExpressionInfo right);
  };
} // namespace phantom

#endif // !PHANTOM_OPERATION_HPP
