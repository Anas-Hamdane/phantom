#include <Driver.hpp>
#include <global.hpp>

namespace phantom {

  void Driver::print_help(bool with_exit) {
    std::cout << "Usage:\n"
                 "   phantom [OPTIONS] file...\n\n"
                 " Available options:\n"
                 "   -o [ARG output_file_path]: specify the output file [DEFAULT = \"a.out\"]\n"
                 "   -O [ARG: ON/OFF]: turn optimization on/off [DEFAULT = on]\n\n"
                 "   --emit-exec: output executable file [DEFAULT]\n"
                 "   --emit-llvm: output llvm IR code\n"
                 "   --emit-asm: output assembly code\n"
                 "   --emit-obj: output object file\n\n"
                 "   --help: print help\n";

    if (with_exit)
      exit(0);
  }

  Options Driver::parse_options() {
    Options result;
    bool source_file_specified = false;

    // the first cmd line argument shouldn't be given
    for (size_t i = 0; i < argv.size(); ++i) {
      std::string arg = argv[i];

      if (arg == "-o") {
        if (i + 1 >= argv.size())
          Report("Expected [ARG: output_file_path] after options: -o", true);

        std::string output_file_name = argv[i + 1];
        if (output_file_name.rfind('-', 0) == 0)
          Report("Incorrect [ARG: output_file_path] form after option: -o, got " + output_file_name, true);

        result.out_file = output_file_name;
        i++;
      } else if (arg == "-O") {
        if (i + 1 >= argv.size())
          Report("Expected [ARG: ON/OFF] after options: -O", true);

        std::string output_file_name = argv[i + 1];
        if (output_file_name != "ON" && output_file_name != "OFF")
          Report("Incorrect [ARG: ON/OFF] form after option: -O, got " + output_file_name, true);

        result.opitimize = (output_file_name == "ON");
        i++;
      } else if (arg.rfind('-', 0) != 0 && !source_file_specified) {
        result.source_file = arg;
        source_file_specified = true;
      } else if (arg == "--emit-llvm")
        result.out_type = "ir";
      else if (arg == "--emit-asm")
        result.out_type = "asm";
      else if (arg == "--emit-obj")
        result.out_type = "obj";
      else if (arg == "--emit-exec")
        result.out_type = "exec";
      else if (arg == "--help")
        print_help();
      else
        Report("Unreconized [OPTION/ARGUMENT] " + arg + "\n", true);
    }

    if (result.source_file.empty())
      Report("Source file required for compilation\n", true);

    return result;
  }
} // namespace phantom
