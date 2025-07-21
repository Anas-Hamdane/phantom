#include "llvm/IR/Type.h"
#include "llvm/IR/Value.h"
#include <llvm_codegen/Help.hpp>
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

      else {
        logger.log(Logger::Level::ERROR, "Invalid Operation casting '" + Help::llvm_to_string(src->getType(), logger) + "' to '" + Help::llvm_to_string(type, logger));
        return nullptr;
      }
    }

    llvm::Value* Operation::resolve_value(Value info) {
      switch (info.type) {
        case Value::Type::Operation:
        case Value::Type::Constant:
          return info.value;
          break;
        case Value::Type::Alloca:
          return builder->CreateLoad(info.pointee->type, info.value);
          break;
        default:
          logger.log(Logger::Level::ERROR, "Unexpected initializer type");
          return nullptr;
          break;
      }
    }

    void Operation::prec_cast(Value& left, Value& right) {
      llvm::Type* left_type = left.value->getType();
      llvm::Type* right_type = right.value->getType();

      int left_prec = type_precedence(left_type);
      int right_prec = type_precedence(right_type);

      if (left_prec > right_prec)
        right.value = cast(right.value, left_type);

      else
        left.value = cast(left.value, right_type);
    }

    // TODO: implement them and simplify codegen
    Value Operation::load(llvm::Value* value, llvm::Type* type) {}
    Value Operation::store(llvm::Value* left, llvm::Value* ptr) {}
    Value Operation::call(llvm::Function* fn, std::vector<llvm::Value*> args) {}

    Value Operation::add(Value left, Value right) {
      const auto value_type = Value::Type::Operation;

      left.value = resolve_value(left);
      right.value = resolve_value(right);

      llvm::Type* left_type = left.value->getType();
      llvm::Type* right_type = right.value->getType();

      if (!left.value || !right.value) {
        logger.log(Logger::Level::ERROR, "Invalid pointer arithmetic: addition for null values", true);
        return {};
      }

      if (left_type->isFloatingPointTy() && right_type->isPointerTy() ||
          left_type->isPointerTy() && right_type->isFloatingPointTy()) {
        logger.log(Logger::Level::ERROR, "Invalid pointer arithmetic expression: (ptr +- f) or (f +- ptr) is undefined", true);
        return {};
    }

      else if (left_type->isPointerTy() && right_type->isPointerTy()) {
        logger.log(Logger::Level::ERROR, "Invalid pointer arithmetic expression: ptr + ptr is undefined", true);
        return {};
      }

      else if (left_type->isIntegerTy() && right_type->isIntegerTy()) {
        if (left.type != right.type)
          prec_cast(left, right);

        llvm::Value* result = builder->CreateAdd(left.value, right.value);
        return {result, value_type};
      }

      else if (left_type->isFloatingPointTy() || right_type->isFloatingPointTy()) {
        if (left.type != right.type)
          prec_cast(left, right);

        llvm::Value* result = builder->CreateFAdd(left.value, right.value);
        return {result, value_type};
      }

      else if (left_type->isPointerTy() && right_type->isIntegerTy()) {
        llvm::Type* pointee_type = left.pointee->type;

        if (!pointee_type)
          logger.log(Logger::Level::ERROR, "Internal compiler error: Unexpected pointer-to type", true);

        llvm::Value* result = builder->CreateGEP(pointee_type, left.value, right.value);
        return {result, value_type};
      }

      else if (left_type->isIntegerTy() && right_type->isPointerTy()) {
        llvm::Type* pointee_type = right.pointee->type;

        if (!pointee_type)
          logger.log(Logger::Level::ERROR, "Internal compiler error: Unexpected pointer-to type", true);

        llvm::Value* result = builder->CreateGEP(pointee_type, right.value, left.value);
        return {result, value_type};
      }

      logger.log(Logger::Level::ERROR, "Unhandled type for an addition sides", true);
      return {};
    }

    Value Operation::sub(Value left, Value right) {
      const auto value_type = Value::Type::Operation;

      left.value = resolve_value(left);
      right.value = resolve_value(right);

      llvm::Type* left_type = left.value->getType();
      llvm::Type* right_type = right.value->getType();

      if (!left.value || !right.value)
        logger.log(Logger::Level::ERROR, "Invalid pointer arithmetic: substraction for null values", true);

      if (left_type->isFloatingPointTy() && right_type->isPointerTy() ||
          left_type->isPointerTy() && right_type->isFloatingPointTy())
        logger.log(Logger::Level::ERROR, "Invalid pointer arithmetic expression: (ptr +- f) or (f +- ptr) is undefined", true);

      else if (left_type->isIntegerTy() && right_type->isIntegerTy()) {
        if (left.type != right.type)
          prec_cast(left, right);

        llvm::Value* result = builder->CreateSub(left.value, right.value);
        return {result, value_type};
      }

      else if (left_type->isFloatingPointTy() || right_type->isFloatingPointTy()) {
        if (left.type != right.type)
          prec_cast(left, right);

        llvm::Value* result = builder->CreateFSub(left.value, right.value);
        return {result, value_type};
      }

      // TODO: handle distance between pointers: ptr - ptr, note that
      // they should point to the same object/array, or at least the
      // same data type (ptr_to_type)
      else if (left_type->isPointerTy() && right_type->isPointerTy())
        logger.log(Logger::Level::ERROR, "ptr - ptr is not implemented yet", true);

      else if (left_type->isPointerTy() && right_type->isIntegerTy()) {
        llvm::Type* pointee_type = left.pointee->type;

        if (!pointee_type)
          logger.log(Logger::Level::ERROR, "Internal compiler error: Unexpected pointer-to type", true);

        llvm::Value* result = builder->CreateGEP(pointee_type, left.value, builder->CreateNeg(right.value));
        return {result, value_type};
      }

      else if (left_type->isIntegerTy() && right_type->isPointerTy())
        logger.log(Logger::Level::ERROR, "Invalid pointer arithmetic expression: n - ptr is undefined", true);

      logger.log(Logger::Level::ERROR, "Unhandled type for substraction expression sides", true);
      return {};
    }

    Value Operation::mul(Value left, Value right) {
      const auto value_type = Value::Type::Operation;

      left.value = resolve_value(left);
      right.value = resolve_value(right);

      llvm::Type* left_type = left.value->getType();
      llvm::Type* right_type = right.value->getType();

      if (!left.value || !right.value)
        logger.log(Logger::Level::ERROR, "Invalid pointer arithmetic: multiplication for null values", true);

      if (left_type->isPointerTy() || right_type->isPointerTy())
        logger.log(Logger::Level::ERROR, "Invalid pointer arithmetic expression: ptr * n/f/ptr is undefined", true);

      else if (left_type->isIntegerTy() && right_type->isIntegerTy()) {
        if (left.type != right.type)
          prec_cast(left, right);

        llvm::Value* result = builder->CreateMul(left.value, right.value);
        return {result, value_type};
      }

      else if (left_type->isFloatingPointTy() || right_type->isFloatingPointTy()) {
        if (left.type != right.type)
          prec_cast(left, right);

        llvm::Value* result = builder->CreateFMul(left.value, right.value);
        return {result, value_type};
      }

      logger.log(Logger::Level::ERROR, "Unhandled type for multiplication expression sides", true);
      return {};
    }

    Value Operation::div(Value left, Value right) {
      const auto value_type = Value::Type::Operation;

      left.value = resolve_value(left);
      right.value = resolve_value(right);

      llvm::Type* left_type = left.value->getType();
      llvm::Type* right_type = right.value->getType();

      if (!left.value || !right.value)
        logger.log(Logger::Level::ERROR, "Invalid pointer arithmetic: division for null values", true);

      if (left_type->isPointerTy() || right_type->isPointerTy())
        logger.log(Logger::Level::ERROR, "Invalid pointer arithmetic expression: ptr / n/f/ptr is undefined", true);

      else if (left_type->isIntegerTy() && right_type->isIntegerTy()) {
        if (left.type != right.type)
          prec_cast(left, right);

        llvm::Value* result = builder->CreateSDiv(left.value, right.value);
        return {result, value_type};
      }

      else if (left_type->isFloatingPointTy() || right_type->isFloatingPointTy()) {
        if (left.type != right.type)
          prec_cast(left, right);

        llvm::Value* result = builder->CreateFDiv(left.value, right.value);
        return {result, value_type};
      }

      logger.log(Logger::Level::ERROR, "Unhandled type for division expression sides", true);
      return {};
    }

    // left: the result of an lvalue() function
    // right: the result of an rvalue() function
    // TODO: revise
    Value Operation::asgn(Value left, Value right) {
      llvm::Value* right_value = resolve_value(right);

      if (!left.value || !right.value || right_value)
        logger.log(Logger::Level::ERROR, "Unexpected assignment expression", true);

      builder->CreateStore(right_value, left.value);
      return right;
    }
  } // namespace llvm_codegen
} // namespace phantom
