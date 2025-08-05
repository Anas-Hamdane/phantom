#pragma once

#include <string>
#include <variant>
#include <vector>
using uint = unsigned int;

namespace phantom {
  namespace ir {
    struct Type {
      enum class Kind { Int, Float } kind;
      uint size;
      bool is_void;
    };

    struct PhysReg {
      std::string name; // register name
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
      std::variant<VirtReg, PhysReg> dst;
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

    // i32 -> f32
    struct Int2Float {
      Value value;
      PhysReg dst;
    };

    // i32 -> f64
    struct Int2Double {
      Value value;
      PhysReg dst;
    };

    // f32 -> i32
    struct Float2Int {
      Value value;
      PhysReg dst;
    };

    // f32 -> f64
    struct Float2Double {
      Value value;
      PhysReg dst;
    };

    // f64 -> i32
    struct Double2Int {
      Value value;
      PhysReg dst;
    };

    // f64 -> f32
    struct Double2Float {
      Value value;
      PhysReg dst;
    };


    using Instruction = std::variant<Alloca, Store, BinOp, UnOp,
                                     Int2Float, Int2Double, Float2Int, Float2Double, Double2Int, Double2Float>;

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
