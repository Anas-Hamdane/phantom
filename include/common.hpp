#ifndef PHANTOM_COMMON_HPP
#define PHANTOM_COMMON_HPP

#include <cstdint>

#include "llvm_codegen/Visitor.hpp"
#include "llvm_codegen/data/Value.hpp"

namespace phantom {
  using Value = llvm_codegen::Value;
  using Visitor = llvm_codegen::Visitor;

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
  constexpr double DOUBLE_MIN_VAL = (-DOUBLE_MAX_VAL);

  // 128-bit float (quad)
  constexpr long double QUAD_MAX_VAL = 1.189731495357231765e+4932Q;
  constexpr long double QUAD_MIN_VAL = (-QUAD_MAX_VAL);

  constexpr char compiler_metadata[] = "phantom";
} // namespace phantom

#endif // !PHANTOM_COMMON_HPP
