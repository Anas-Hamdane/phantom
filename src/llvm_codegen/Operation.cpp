#include "llvm/IR/Type.h"
#include "llvm/IR/Value.h"
#include <llvm_codegen/Operation.hpp>

#include <common.hpp>

namespace phantom {
  namespace llvm_codegen {
    int Operation::type_precedence(llvm::Type* type) {
      // clang-format off
    if (type->isIntegerTy()) {
      switch (type->getIntegerBitWidth()) {
        case 1:   return 1; // bool
        case 8:   return 2; // char
        case 16:  return 3; // short
        case 32:  return 4; // int
        case 64:  return 5; // long
        case 128: return 5; // huge (not implemented, for futur support)
        default: return 0;
      }
    }

    if (type->isFloatTy())  return 10;
    if (type->isDoubleTy()) return 11;
    if (type->isFP128Ty())  return 12;

    return 0;
      // clang-format on
    }

    llvm::Value* Operation::cast(llvm::Value* src, llvm::Type* type) {
      if (src->getType() == type)
        return src;

      if (src->getType()->isIntegerTy() && type->isIntegerTy())
        return builder->CreateIntCast(src, type, true);

      else if (src->getType()->isIntegerTy() && type->isFloatingPointTy())
        return builder->CreateSIToFP(src, type);

      else if (src->getType()->isFloatingPointTy() && type->isIntegerTy())
        return builder->CreateFPToSI(src, type);

      else if (src->getType()->isFloatingPointTy() && type->isFloatingPointTy())
        return builder->CreateFPCast(src, type);

      else if (src->getType()->isPointerTy() && type->isPointerTy())
        return builder->CreateBitCast(src, type);

      else
        return nullptr;
    }

    void Operation::prec_cast(ExprInfo& left, ExprInfo& right) {
      int left_prec = type_precedence(left.type);
      int right_prec = type_precedence(right.type);

      if (left_prec > right_prec)
        right.value = cast(right.value, left.type);

      else
        left.value = cast(left.value, right.type);
    }

    ExprInfo Operation::add(ExprInfo left, ExprInfo right) {
      if (!left.value || !right.value)
        logger.log(Logger::Level::ERROR, "Invalid pointer arithmetic: addition for null values", true);

      if (left.type->isFloatingPointTy() && right.type->isPointerTy() ||
          left.type->isPointerTy() && right.type->isFloatingPointTy())
        logger.log(Logger::Level::ERROR, "Invalid pointer arithmetic expression: (ptr +- f) or (f +- ptr) is undefined", true);

      else if (left.type->isPointerTy() && right.type->isPointerTy())
        logger.log(Logger::Level::ERROR, "Invalid pointer arithmetic expression: ptr + ptr is undefined", true);

      else if (left.type->isIntegerTy() && right.type->isIntegerTy()) {
        if (left.type != right.type)
          prec_cast(left, right);

        llvm::Value* res = builder->CreateAdd(left.value, right.value);
        return {res, res->getType()};
      }

      else if (left.type->isFloatingPointTy() || right.type->isFloatingPointTy()) {
        if (left.type != right.type)
          prec_cast(left, right);

        llvm::Value* res = builder->CreateFAdd(left.value, right.value);
        return {res, res->getType()};
      }

      else if (left.type->isPointerTy() && right.type->isIntegerTy()) {
        llvm::Type* ptr_to_type = left.variable->ptr_to_variable->type;

        if (!ptr_to_type)
          logger.log(Logger::Level::ERROR, "Internal compiler error: Unexpected pointer-to type", true);

        llvm::Value* res = builder->CreateGEP(ptr_to_type, left.value, right.value);
        return {res, res->getType()};
      }

      else if (left.type->isIntegerTy() && right.type->isPointerTy()) {
        llvm::Type* ptr_to_type = right.variable->ptr_to_variable->type;

        if (!ptr_to_type)
          logger.log(Logger::Level::ERROR, "Internal compiler error: Unexpected pointer-to type", true);

        llvm::Value* res = builder->CreateGEP(ptr_to_type, right.value, left.value);
        return {res, res->getType()};
      }

      else
        logger.log(Logger::Level::ERROR, "Unhandled type for an addition sides", true);

      return {};
    }

    ExprInfo Operation::sub(ExprInfo left, ExprInfo right) {
      if (!left.value || !right.value)
        logger.log(Logger::Level::ERROR, "Invalid pointer arithmetic: substraction for null values", true);

      if (left.type->isFloatingPointTy() && right.type->isPointerTy() ||
          left.type->isPointerTy() && right.type->isFloatingPointTy())
        logger.log(Logger::Level::ERROR, "Invalid pointer arithmetic expression: (ptr +- f) or (f +- ptr) is undefined", true);

      else if (left.type->isIntegerTy() && right.type->isIntegerTy()) {
        if (left.type != right.type)
          prec_cast(left, right);

        llvm::Value* res = builder->CreateSub(left.value, right.value);
        return {res, res->getType()};
      }

      else if (left.type->isFloatingPointTy() || right.type->isFloatingPointTy()) {
        if (left.type != right.type)
          prec_cast(left, right);

        llvm::Value* res = builder->CreateFSub(left.value, right.value);
        return {res, res->getType()};
      }

      // TODO: handle distance between pointers: ptr - ptr, note that
      // they should point to the same object/array, or at least the
      // same data type (ptr_to_type)
      else if (left.type->isPointerTy() && right.type->isPointerTy())
        logger.log(Logger::Level::ERROR, "ptr - ptr is not implemented yet", true);

      else if (left.type->isPointerTy() && right.type->isIntegerTy()) {
        llvm::Type* ptr_to_type = left.variable->ptr_to_variable->type;

        if (!ptr_to_type)
          logger.log(Logger::Level::ERROR, "Internal compiler error: Unexpected pointer-to type", true);

        llvm::Value* res = builder->CreateGEP(ptr_to_type, left.value, builder->CreateNeg(right.value));
        return {res, res->getType()};
      }

      else if (left.type->isIntegerTy() && right.type->isPointerTy())
        logger.log(Logger::Level::ERROR, "Invalid pointer arithmetic expression: n - ptr is undefined", true);

      else
        logger.log(Logger::Level::ERROR, "Unhandled type for substraction expression sides", true);

      return {};
    }

    ExprInfo Operation::mul(ExprInfo left, ExprInfo right) {
      if (!left.value || !right.value)
        logger.log(Logger::Level::ERROR, "Invalid pointer arithmetic: multiplication for null values", true);

      if (left.type->isPointerTy() || right.type->isPointerTy())
        logger.log(Logger::Level::ERROR, "Invalid pointer arithmetic expression: ptr * n/f/ptr is undefined", true);

      else if (left.type->isIntegerTy() && right.type->isIntegerTy()) {
        if (left.type != right.type)
          prec_cast(left, right);

        llvm::Value* res = builder->CreateMul(left.value, right.value);
        return {res, res->getType()};
      }

      else if (left.type->isFloatingPointTy() || right.type->isFloatingPointTy()) {
        if (left.type != right.type)
          prec_cast(left, right);

        llvm::Value* res = builder->CreateFMul(left.value, right.value);
        return {res, res->getType()};
      }

      else
        logger.log(Logger::Level::ERROR, "Unhandled type for multiplication expression sides", true);

      return {};
    }

    ExprInfo Operation::div(ExprInfo left, ExprInfo right) {
      if (!left.value || !right.value)
        logger.log(Logger::Level::ERROR, "Invalid pointer arithmetic: division for null values", true);

      if (left.type->isPointerTy() || right.type->isPointerTy())
        logger.log(Logger::Level::ERROR, "Invalid pointer arithmetic expression: ptr / n/f/ptr is undefined", true);

      else if (left.type->isIntegerTy() && right.type->isIntegerTy()) {
        if (left.type != right.type)
          prec_cast(left, right);

        llvm::Value* res = builder->CreateSDiv(left.value, right.value);
        return {res, res->getType()};
      }

      else if (left.type->isFloatingPointTy() || right.type->isFloatingPointTy()) {
        if (left.type != right.type)
          prec_cast(left, right);

        llvm::Value* res = builder->CreateFDiv(left.value, right.value);
        return {res, res->getType()};
      }

      else
        logger.log(Logger::Level::ERROR, "Unhandled type for division expression sides", true);

      return {};
    }

    // left: the result of an lvalue() function
    // right: the result of an rvalue() function
    ExprInfo Operation::asgn(ExprInfo left, ExprInfo right) {
      if (!left.value || !right.value)
        logger.log(Logger::Level::ERROR, "Unexpected assignment expression", true);

      builder->CreateStore(right.value, left.value);
      return right;
    }
  } // namespace llvm_codegen
} // namespace phantom
