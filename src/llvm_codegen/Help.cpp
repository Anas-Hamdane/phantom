#include "llvm/IR/DerivedTypes.h"
#include <llvm_codegen/Help.hpp>

namespace phantom {
  namespace llvm_codegen {
    std::string Help::llvm_to_string(llvm::Type* type, const Logger& logger) {
      if (type->isPointerTy())
        return "ptr";
      else if (type->isIntegerTy(1))
        return "bool";
      else if (type->isIntegerTy(8))
        return "char";
      else if (type->isIntegerTy(16))
        return "short";
      else if (type->isIntegerTy(32))
        return "int";
      else if (type->isIntegerTy(64))
        return "long";
      else if (type->isFP128Ty())
        return "quad";
      else if (type->isDoubleTy())
        return "double";
      else if (type->isFloatTy())
        return "float";
      else if (type->isVoidTy())
        return "void";
      else
        logger.log(Logger::Level::FATAL, "Unexpected llvm type", true);

      return "";
    }

    llvm::Type* Help::string_to_llvm(std::string type, std::shared_ptr<llvm::LLVMContext> context, const Logger& logger) {
      if (type == "ptr")
        return llvm::PointerType::get(*context, 0);
      else if (type == "void")
        return llvm::Type::getVoidTy(*context);
      else if (type == "bool")
        return llvm::Type::getInt1Ty(*context);
      else if (type == "char")
        return llvm::Type::getInt8Ty(*context);
      else if (type == "short")
        return llvm::Type::getInt16Ty(*context);
      else if (type == "int")
        return llvm::Type::getInt32Ty(*context);
      else if (type == "long")
        return llvm::Type::getInt64Ty(*context);
      else if (type == "float")
        return llvm::Type::getFloatTy(*context);
      else if (type == "double")
        return llvm::Type::getDoubleTy(*context);
      else if (type == "quad")
        return llvm::Type::getFP128Ty(*context);
      else
        logger.log(Logger::Level::FATAL, "Unidentified data type: '" + type + "'", true);

      return nullptr;
    }

    TypeInfo::Kind Help::llvm_to_kind(llvm::Type* type, const Logger& logger) {
      if (type->isPointerTy())
        return TypeInfo::Kind::Ptr;
      else if (type->isArrayTy())
        return TypeInfo::Kind::Array;
      else if (type->isIntegerTy(1))
        return TypeInfo::Kind::Bool;
      else if (type->isIntegerTy(8))
        return TypeInfo::Kind::Char;
      else if (type->isIntegerTy(16))
        return TypeInfo::Kind::Short;
      else if (type->isIntegerTy(32))
        return TypeInfo::Kind::Int;
      else if (type->isIntegerTy(64))
        return TypeInfo::Kind::Long;
      // else if (type->isIntegerTy(128))
      //   return TypeInfo::Kind::Huge;
      else if (type->isFP128Ty())
        return TypeInfo::Kind::Quad;
      else if (type->isDoubleTy())
        return TypeInfo::Kind::Double;
      else if (type->isFloatTy())
        return TypeInfo::Kind::Float;
      else if (type->isVoidTy())
        return TypeInfo::Kind::Void;
      else
        logger.log(Logger::Level::FATAL, "Unexpected llvm type", true);

      return TypeInfo::Kind::BAKA;
    }

    llvm::Value* Help::default_return(llvm::Type* type, std::shared_ptr<llvm::IRBuilder<>> builder) {
      return builder->CreateRet(llvm::Constant::getNullValue(type));
    }
  } // namespace llvm_codegen
} // namespace phantom
