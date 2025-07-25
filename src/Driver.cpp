#include <Driver.hpp>
#include <info.hpp>

namespace phantom {
  void Driver::print_help(const std::string& program_name) {
    // clang-format off
    static std::string help = "Usage:\n"
      "   " + program_name + " [OPTIONS] file...\n\n"
      " Available options:\n"
      "   -o [output_file_path]:\n"
      "       specify the output file [DEFAULT = \"a.out\"]\n"
      "   -O [ON|OFF]:\n"
      "      turn optimization on/off [DEFAULT = on]\n\n"
      "   --emit [llvm-ir|asm|obj]:\n"
      "      type of the output file\n\n"
      "   --print [tokens]:\n"
      "      print the options to stdout\n\n"
      "   --color [ON|OFF]:\n"
      "      colored log output [DEFAULT = ON]\n"
      "   --help:\n"
      "      print help\n";

    fwrite(help.c_str(), 1, help.length(), stdout);
    exit(0);
  }
  // clang-format on

  Options Driver::parse_options() {
    Options opts;
    bool source_file_specified = false;

    opts.program_name = argv[0];

    for (size_t i = 1; i < argv.size(); ++i) {
      std::string arg = argv[i];

      if (arg == "-o") {
        if (i + 1 >= argv.size())
          logger.log(Logger::Level::FATAL, "Expected [output_file_path] after \"-o\"", true);

        std::string output_file_name = argv[i + 1];
        if (output_file_name.rfind('-', 0) == 0)
          logger.log(Logger::Level::FATAL, "Incorrect [output_file_path] form \"-o\", got " + output_file_name, true);

        opts.output_file = output_file_name;
        i++;
      } else if (arg == "-O") {
        if (i + 1 >= argv.size())
          logger.log(Logger::Level::FATAL, "Expected [ON|OFF] after \"-O\"", true);

        std::string toggle = argv[i + 1];
        if (toggle != "ON" && toggle != "OFF")
          logger.log(Logger::Level::FATAL, "Incorrect [ON|OFF] form after \"-O\", got " + toggle, true);

        opts.opitimize = (toggle == "ON");
        i++;
      } else if (arg == "--color") {
        if (i + 1 >= argv.size())
          logger.log(Logger::Level::FATAL, "Expected [ON|OFF] after \"--color\"", true);

        std::string toggle = argv[i + 1];
        if (toggle != "ON" && toggle != "OFF")
          logger.log(Logger::Level::FATAL, "Incorrect [ON|OFF] form after \"--color\", got " + toggle, true);

        opts.log_color = (toggle == "ON");
        logger.colored = (toggle == "ON");
        i++;
      } else if (arg == "--emit") {
        if (i + 1 >= argv.size())
          logger.log(Logger::Level::FATAL, "Expected [llvm-ir|asm|obj] after \"--emit\"", true);

        std::string type = argv[i + 1];
        if (type != "llvm-ir" && type != "asm" && type != "obj")
          logger.log(Logger::Level::FATAL, "Incorrect [llvm-ir|asm|obj] form after \"--emit\", got " + type, true);

        opts.out_type = type;
        i++;
      } else if (arg == "--print") {
        if (i + 1 >= argv.size())
          logger.log(Logger::Level::FATAL, "Expected [tokens] after \"--print\"", true);

        std::string option = argv[i + 1];
        if (option != "tokens")
          logger.log(Logger::Level::FATAL, "Incorrect [tokens] form after \"-print\", got " + option, true);

        opts.print = option;
        i++;
      } else if (arg.rfind('-', 0) != 0 && !source_file_specified) {
        opts.source_file = arg;
        source_file_specified = true;
      } else if (arg == "--help")
        print_help(opts.program_name);
      else
        logger.log(Logger::Level::FATAL, "Unreconized [OPTION/ARGUMENT] " + arg + "\n", true);
    }

    if (opts.source_file.empty())
      logger.log(Logger::Level::FATAL, "Source file is required for compilation", true);

    return opts;
  }
} // namespace phantom
