#pragma once

#include <cstdint>

namespace phantom {
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

  // 32-bit float
  constexpr float FLOAT_MAX_VAL = 3.402823466e+38F;
  constexpr float FLOAT_MIN_VAL = -FLOAT_MAX_VAL;

  // 64-bit float
  constexpr double DOUBLE_MAX_VAL = 1.7976931348623157e+308;
  constexpr double DOUBLE_MIN_VAL = -DOUBLE_MAX_VAL;

  // floating point fraction max digits
  constexpr int FP_FRACTION_MD = 18;
  constexpr unsigned int STR_INIT = 256;
  constexpr unsigned int VEC_INIT = 10;

  constexpr char compiler_metadata[] = "phantom";

  struct Type {
    enum class Kind {
      Void,
      Int,
      UnsInt,
      FP
    } kind;
    uint8_t bitwidth;
  };

  [[noreturn]] void __unreachable__impl(const char* file, int line, const char* func);
  #define unreachable() __unreachable__impl(__FILE__, __LINE__, __func__)

  [[noreturn]] void __todo__impl(const char* file, int line, const char* func);
  #define todo() __todo__impl(__FILE__, __LINE__, __func__)
} // namespace phantom
