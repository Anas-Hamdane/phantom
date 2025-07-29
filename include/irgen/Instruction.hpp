#pragma once

#include "Value.hpp"

namespace phantom {
  namespace irgen {
    enum class InstKind {
      Alloca,
      Store,
      Load,
      Return
    };

    struct BasicBlock;
    struct FnDecl {
      Function fn;
    };
    struct FnDef {
      Function fn;

      BasicBlock* blocks;
      size_t blocks_size;
    };
    struct Alloca {
      const char* name;
      Type type;
    };
    struct Store {
      Value src;
      Register dst;
    };
    struct Load {
      Value src;
      Register dst;
    };
    struct Return {
      Value value;
    };

    struct Instruction {
      InstKind kind;

      union {
        Alloca alloca;
        Store store;
        Load load;
        Return ret;
      } inst;
    };

    struct BasicBlock {
      const char* name;
      unsigned int id;

      Instruction* instructions;
      size_t insts_len;

      Instruction terminator;
    };
  } // namespace irgen
} // namespace phantom
