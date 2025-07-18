#ifndef PHANTOM_DRIVER_HPP
#define PHANTOM_DRIVER_HPP

#include "Options.hpp"
#include <Logger.hpp>
// #include <array>
#include <vector>

namespace phantom {

  class Driver {
    const std::vector<std::string> argv;
    Logger& logger;
    size_t index = 0;

    // inline static constexpr std::array<std::pair<std::string_view, std::array<std::string_view, 3>>, 6> cmdline_args = {{
    //     {"-o", {{"", "", ""}}},
    //     {"-O", {{"ON", "OFF", ""}}},
    //     {"--emit", {{"llvm-ir", "asm", "obj"}}},
    //     {"--print", {{"tokens", "", ""}}},
    //     {"--help", {{"", "", ""}}},
    //     {"--color", {{"ON", "OFF", ""}}},
    // }};

    void print_help(const std::string& program_name);
    // void parse_option(Options& opts);
    // std::string peek(off_t offset = 0);
    // std::string consume(off_t offset = 1);

public:
    Driver(const std::vector<std::string>& argv, Logger& logger) : argv(argv), logger(logger) {}
    Options parse_options();
  };
} // namespace phantom

#endif // !PHANTOM_DRIVER_HPP
