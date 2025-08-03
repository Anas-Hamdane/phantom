#pragma once

#include "common.hpp"
#include <string>
#include <variant>
#include <vector>
using uint = unsigned int;

namespace phantom {
  namespace ir {
    struct PhysReg {
      char name;
      Type type;
    };
    struct VirtReg {
      uint id;
      Type type;
    };
    struct Constant {
      Type type;
      std::variant<int64_t, double> value;
    };

    using Value = std::variant<Constant, VirtReg, PhysReg>;

    struct Return {
      Value value;
    };
    using Terminator = std::variant<Return>;

    struct Alloca {
      Type type;
      VirtReg reg;
    };
    struct Store {
      Value src;
      VirtReg dst;
    };
    struct BinOp {
      // clang-format off
      enum class Op { Add, Sub, Mul, Div } op;
      Value lhs, rhs;
      PhysReg dst;
      // clang-format on
    };
    struct UnOp {
      // clang-format off
      enum class Op { Neg, Not } op;
      Value operand;
      PhysReg dst;
      // clang-format on
    };
    using Instruction = std::variant<Alloca, Store, BinOp, UnOp>;

    struct Function {
      std::string name;
      Type return_type;
      std::vector<VirtReg> params;
      std::vector<Instruction> body;
      Terminator terminator;
      bool terminated = false;
      bool defined = false;
    };

    struct GlobalVariable {
      std::string name;
      Type type;
      Constant init;
      bool externed = false;
    };

    struct Target {
      std::string arch;
      std::string kernel;
    };

    struct Program {
      Target target;
      std::vector<Function> funcs;
      std::vector<GlobalVariable> globals;
    };
  } // namespace ir
} // namespace phantom
