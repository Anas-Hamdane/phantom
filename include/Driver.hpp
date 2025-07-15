#ifndef PHANTOM_DRIVER_HPP
#define PHANTOM_DRIVER_HPP

#include <Options.hpp>
#include <vector>

/*
 * should be called like this:
 *   phantom [OPTIONS] file...
 *
 * Available options:
 *   -o [ARG output_file_path]: specify the output file [DEFAULT = "a.out"]
 *   -O [ARG: ON/OFF]: turn optimization on/off [DEFAULT = on]
 *
 *   --emit-exec: output executable file [DEFAULT]
 *   --emit-llvm: output llvm IR code
 *   --emit-asm: output assembly code
 *   --emit-obj: output object file
 *
 *   --help: prints this help
 */

namespace phantom {
  class Driver {
    std::vector<std::string> argv;

    void print_help(bool with_exit = true);

public:
    Driver(const std::vector<std::string>& argv) : argv(std::move(argv)) {}
    Options parse_options();
  };
} // namespace phantom

#endif // !PHANTOM_DRIVER_HPP
