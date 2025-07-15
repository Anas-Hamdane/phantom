#ifndef PHANTOM_OPTIONS_HPP
#define PHANTOM_OPTIONS_HPP

#include <string>

namespace phantom {
  struct Options {
    // the source file
    std::string source_file;

    // the output file specified by -o
    std::string out_file = "a.out";

    // avialable types
    // {"ir", "asm", "obj", "exec"}
    std::string out_type = "exec";

    // optimize or not
    bool opitimize = true;
  };
} // namespace phantom

#endif // !PHANTOM_OPTIONS_HPP
