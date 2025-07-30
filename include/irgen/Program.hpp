#pragma once

#include "utils/vec.hpp"

namespace phantom {
  namespace ir {
    struct Register {
      uint id;
      Type type;
    };

    struct Constant {
      Type type;

      union {
        uint64_t int_val;
        double float_val;
        bool bool_val;
      } value;
    };

    enum class ValueKind {
      Register,
      Constant
    };
    struct Value {
      ValueKind kind;

      union {
        Register reg;
        Constant con;
      } value;
    };

    struct Return {
      Value value;
    };

    enum class TermKind {
      Return
    };
    struct Terminator {
      TermKind kind;

      union {
        Return ret;
      } data;
    };

    struct Alloca {
      Type type;
      Register reg;
    };
    struct Store {
      Value src;
      Register dst;
    };
    struct Load {
      Register src;
      Register dst;
    };

    struct BinOp {
      // clang-format off
      enum class Op { Add, Sub, Mul, Div } op;
      Value lhs, rhs;
      Register dst;
      // clang-format on
    };

    struct UnOp  {
      // clang-format off
      enum class Op { Neg, Not } op;
      Value operand;
      Register dst;
      // clang-format on
    };

    enum class InstrKind {
      Alloca,
      Store,
      Load,
      BinOp,
      UnOp
    };
    struct Instruction {
      InstrKind kind;

      union {
        Alloca alloca;
        Store store;
        Load load;
        BinOp binop;
        UnOp unop;
      } inst;
    };

    struct BasicBlock {
      uint id;
      utils::Vec<Instruction> insts;
      Terminator terminator;
      bool terminated = false;
    };

    struct Function {
      const char* name;
      Type return_type;
      utils::Vec<Register> params;
      utils::Vec<BasicBlock> blocks;
      bool defined = false;
      bool externed = false;
    };

    struct GlobalVariable {
      const char* name;
      Type type;
      Constant init;
      bool externed = false;
    };

    struct Program {
      utils::Vec<Function> funcs;
      utils::Vec<GlobalVariable> globals;
    };
  } // namespace ir
} // namespace phantom
