#include "common.hpp"
#include <cstdio>
#include <cstdlib>

namespace phantom {
  void __todo__impl(const char* file, int line, const char* func) {
    fprintf(stderr, "`todo()` call:\n");
    fprintf(stderr, "feature not yet implemented in function %s at %s:%d", func, file, line);
    std::abort();
  }

  void __unreachable__impl(const char* file, int line, const char* func) {
    fprintf(stderr, "`unreachable()` call in function %s at %s:%d \n", func, file, line);
    std::abort();
  }

  void __torevise__impl(const char* file, int line, const char* func) {
    fprintf(stderr, "`torevise()` call:\n");
    fprintf(stderr, "consider revising function %s at %s:%d\n", func, file, line);
    std::abort();
  }

  char* get_register_by_size(const std::string& reg, int size) {
    // clang-format off
    std::string base = reg;
    if      (reg == "al"  || reg == "ax" || reg == "eax" || reg == "rax") base = "rax";
    else if (reg == "bl"  || reg == "bx" || reg == "ebx" || reg == "rbx") base = "rbx";
    else if (reg == "cl"  || reg == "cx" || reg == "ecx" || reg == "rcx") base = "rcx";
    else if (reg == "dl"  || reg == "dx" || reg == "edx" || reg == "rdx") base = "rdx";
    else if (reg == "spl" || reg == "sp" || reg == "esp" || reg == "rsp") base = "rsp";
    else if (reg == "bpl" || reg == "bp" || reg == "ebp" || reg == "rbp") base = "rbp";
    else if (reg == "sil" || reg == "si" || reg == "esi" || reg == "rsi") base = "rsi";
    else if (reg == "dil" || reg == "di" || reg == "edi" || reg == "rdi") base = "rdi";
    else if (reg.substr(0,2) == "r8")                                     base = "r8";
    else if (reg.substr(0,2) == "r9")                                     base = "r9";
    else if (reg.substr(0,3) == "r10")                                    base = "r10";
    else if (reg.substr(0,3) == "r11")                                    base = "r11";
    else if (reg.substr(0,3) == "r12")                                    base = "r12";
    else if (reg.substr(0,3) == "r13")                                    base = "r13";
    else if (reg.substr(0,3) == "r14")                                    base = "r14";
    else if (reg.substr(0,3) == "r15")                                    base = "r15";

    // Return appropriate size variant
    if (base == "rax") {
      if (size == 1) return (char*) "al";
      if (size == 2) return (char*) "ax";
      if (size == 4) return (char*) "eax";
      if (size == 8) return (char*) "rax";
    }
    else if (base == "rbx") {
      if (size == 1) return (char*) "bl";
      if (size == 2) return (char*) "bx";
      if (size == 4) return (char*) "ebx";
      if (size == 8) return (char*) "rbx";
    }
    else if (base == "rcx") {
      if (size == 1) return (char*) "cl";
      if (size == 2) return (char*) "cx";
      if (size == 4) return (char*) "ecx";
      if (size == 8) return (char*) "rcx";
    }
    else if (base == "rdx") {
      if (size == 1) return (char*) "dl";
      if (size == 2) return (char*) "dx";
      if (size == 4) return (char*) "edx";
      if (size == 8) return (char*) "rdx";
    }
    else if (base == "rsp") {
      if (size == 1) return (char*) "spl";
      if (size == 2) return (char*) "sp";
      if (size == 4) return (char*) "esp";
      if (size == 8) return (char*) "rsp";
    }
    else if (base == "rbp") {
      if (size == 1) return (char*) "bpl";
      if (size == 2) return (char*) "bp";
      if (size == 4) return (char*) "ebp";
      if (size == 8) return (char*) "rbp";
    }
    else if (base == "rsi") {
      if (size == 1) return (char*) "sil";
      if (size == 2) return (char*) "si";
      if (size == 4) return (char*) "esi";
      if (size == 8) return (char*) "rsi";
    }
    else if (base == "rdi") {
      if (size == 1) return (char*) "dil";
      if (size == 2) return (char*) "di";
      if (size == 4) return (char*) "edi";
      if (size == 8) return (char*) "rdi";
    }
    else if (base == "r8") {
      if (size == 1) return (char*) "r8b";
      if (size == 2) return (char*) "r8w";
      if (size == 4) return (char*) "r8d";
      if (size == 8) return (char*) "r8";
    }
    else if (base == "r9") {
      if (size == 1) return (char*) "r9b";
      if (size == 2) return (char*) "r9w";
      if (size == 4) return (char*) "r9d";
      if (size == 8) return (char*) "r9";
    }
    else if (base == "r10") {
      if (size == 1) return (char*) "r10b";
      if (size == 2) return (char*) "r10w";
      if (size == 4) return (char*) "r10d";
      if (size == 8) return (char*) "r10";
    }
    else if (base == "r11") {
      if (size == 1) return (char*) "r11b";
      if (size == 2) return (char*) "r11w";
      if (size == 4) return (char*) "r11d";
      if (size == 8) return (char*) "r11";
    }
    else if (base == "r12") {
      if (size == 1) return (char*) "r12b";
      if (size == 2) return (char*) "r12w";
      if (size == 4) return (char*) "r12d";
      if (size == 8) return (char*) "r12";
    }
    else if (base == "r13") {
      if (size == 1) return (char*) "r13b";
      if (size == 2) return (char*) "r13w";
      if (size == 4) return (char*) "r13d";
      if (size == 8) return (char*) "r13";
    }
    else if (base == "r14") {
      if (size == 1) return (char*) "r14b";
      if (size == 2) return (char*) "r14w";
      if (size == 4) return (char*) "r14d";
      if (size == 8) return (char*) "r14";
    }
    else if (base == "r15") {
      if (size == 1) return (char*) "r15b";
      if (size == 2) return (char*) "r15w";
      if (size == 4) return (char*) "r15d";
      if (size == 8) return (char*) "r15";
    }
    // clang-format on

    unreachable();
  }
} // namespace phantom
