#ifndef PHANTOM_LLVM_CODEGEN_DATA_TYPE_HPP
#define PHANTOM_LLVM_CODEGEN_DATA_TYPE_HPP

#include <memory>
#include <string>

namespace llvm {
  class Value;
  class Type;
} // namespace llvm

namespace phantom {
  namespace llvm_codegen {
    class TypeInfo {
  public:
      enum class Kind {
        Bool,
        Char,
        Short,
        Int,
        Long,
        /* Huge, */
        Float,
        Double,
        Quad,
        Ptr,
        Array,
        Void,

        /*
         * Unknown types, should never happen
         */
        BAKA
      };

      Kind kind;
      std::shared_ptr<TypeInfo> element_type; // arrays/pointer
      size_t array_len;

      TypeInfo(Kind kind, std::shared_ptr<TypeInfo> element_type = nullptr, size_t array_len = 0)
          : kind(kind), element_type(std::move(element_type)), array_len(array_len) {}

      bool isPointer() const { return kind == Kind::Ptr; }
      bool isArray() const { return kind == Kind::Array; }
    };

    struct VarInfo {
      std::string name;
      llvm::Value* value;
      std::shared_ptr<TypeInfo> type;

      VarInfo(const std::string& name, llvm::Value* value, std::shared_ptr<TypeInfo> type)
          : name(name), value(value), type(std::move(type)) {}
    };
  } // namespace llvm_codegen
} // namespace phantom

#endif // !PHANTOM_LLVM_CODEGEN_DATA_TYPE_HPP
