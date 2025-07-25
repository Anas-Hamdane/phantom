#pragma once

#include "Logger.hpp"
#include <vector>

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
  class Driver {
    const std::vector<std::string> argv;
    Logger& logger;

    void print_help(const std::string& program_name);

public:
    Driver(const std::vector<std::string>& argv, Logger& logger) : argv(argv), logger(logger) {}
    Options parse_options();
  };
} // namespace phantom
