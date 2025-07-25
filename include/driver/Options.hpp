#pragma once

#include <string>

namespace phantom {
  struct Options {
    std::string program_name;
    std::string source_file;
    std::string output_file = "a.out";

    // avialable types
    // {"llvm-ir", "asm", "obj"}
    std::string out_type = "";

    // avialable options:
    // {tokens}
    std::string print = "";

    // optimize or not
    bool opitimize = true;
    bool log_color = true;
  };
} // namespace phantom
