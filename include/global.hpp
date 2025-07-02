#ifndef PHANTOM_GLOBAL_HPP
#define PHANTOM_GLOBAL_HPP
/*
 * Header for coloring in error reports
 */

#include <iostream>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>

namespace phantom {
  // 8-bit integer
  constexpr int8_t BYTE_MAX_VAL = 127;
  constexpr int8_t BYTE_MIN_VAL = -128;

  // 16-bit integer
  constexpr short SHORT_MAX_VAL = 32767;
  constexpr short SHORT_MIN_VAL = -32768;

  // 32-bit integer
  constexpr int INT_MAX_VAL = 2147483647;
  constexpr int INT_MIN_VAL = -2147483648;

  // 64-bit integer
  constexpr long LONG_MAX_VAL = 9223372036854775807L;
  constexpr long LONG_MIN_VAL = (-LONG_MAX_VAL - 1L);

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

  // clang-format off
  #if defined(_WIN32) || defined(_WIN64)
    #include <io.h>
    #include <windows.h>

  #elif defined(__unix__) || defined(__unix) || defined(__APPLE__) || defined(__linux__)
    #include <sys/ioctl.h>
    #include <unistd.h>
  #endif

  static bool is_stderr_tty_colored() {
    static bool initialized = false;
    static bool colored = false;

    if (!initialized) {
      #if defined(__unix__) || defined(__unix) || defined(__APPLE__) || defined(__linux__)
            colored = isatty(fileno(stderr));
      #elif defined(_WIN32) || defined(_WIN64)
            colored = _isatty(_fileno(stderr));
      #endif

      initialized = true;
    }
    // clang-format on
    return colored;
  }

  // error reporting
  inline void Report(std::string rep_string, bool error = false) {
    bool colored = is_stderr_tty_colored();

    std::string constructed_msg;

    if (error) {
      if (colored)
        constructed_msg += "\x1b[1;31m";

      constructed_msg += "Error:";

      if (colored)
        constructed_msg += "\x1b[0m";
    } else {
      if (colored)
        constructed_msg += "\x1b[1;33m";

      constructed_msg += "Warning:";

      if (colored)
        constructed_msg += "\x1b[0m";
    }

    constructed_msg += '\n';

    constructed_msg += rep_string;

    std::cerr << constructed_msg;

    if (error)
      exit(EXIT_FAILURE);
  }
} // namespace phantom

#endif // !PHANTOM_GLOBAL_HPP
