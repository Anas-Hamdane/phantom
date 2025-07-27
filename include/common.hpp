#pragma once

#include <cstdint>

// #include "llvm_codegen/data/ExprInfo.hpp"
// #include "llvm_codegen/data/Variable.hpp"
// #include "llvm_codegen/Visitor.hpp"

namespace phantom {
  // using Variable = llvm_codegen::Variable;
  // using ExprInfo = llvm_codegen::ExprInfo;
  // using Visitor = llvm_codegen::Visitor;
  class Variable;
  class ExprInfo;
  class Visitor;

  typedef unsigned int ExprRef;
  typedef unsigned int StmtRef;

  // 8-bit integer
  constexpr int8_t CHAR_MAX_VAL = 127;
  constexpr int8_t CHAR_MIN_VAL = -128;

  // 16-bit integer
  constexpr short SHORT_MAX_VAL = 32767;
  constexpr short SHORT_MIN_VAL = -32768;

  // 32-bit integer
  constexpr int INT_MAX_VAL = 2147483647;
  constexpr int INT_MIN_VAL = -2147483648;

  // 64-bit integer
  constexpr long long LONG_MAX_VAL = 9223372036854775807L;
  constexpr long long LONG_MIN_VAL = (-LONG_MAX_VAL - 1L);

  // 128-bit integer (huge)
  // currently unsupported since there's no way to store
  // such a huge value in c++

  // 32-bit float
  constexpr float FLOAT_MAX_VAL = 3.402823466e+38F;
  constexpr float FLOAT_MIN_VAL = -FLOAT_MAX_VAL;

  // 64-bit float
  constexpr double DOUBLE_MAX_VAL = 1.7976931348623157e+308;
  constexpr double DOUBLE_MIN_VAL = -DOUBLE_MAX_VAL;

  // floating point fraction max digits
  constexpr int FP_FRACTION_MD = 18;

  constexpr char compiler_metadata[] = "phantom";
} // namespace phantom
