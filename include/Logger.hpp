#pragma once

#include <string>

namespace phantom {
  struct Location;
  class Logger {
public:
    bool colored;
    enum Level {
      DEBUG = 0,
      INFO,
      WARNING,
      ERROR,
      FATAL
    };

    Logger(bool colored = true) : colored(colored) {}
    void log(Level level, const std::string& message, const Location& location, const bool exit = false, FILE* stream = stderr) const;
    void log(Level level, const std::string& message, const bool exit = false, FILE* stream = stderr) const;

private:

    // for formatting
    static constexpr const char* RESET = "\033[0m";
    static constexpr const char* RED = "\033[31m";
    static constexpr const char* YELLOW = "\033[33m";
    static constexpr const char* BLUE = "\033[34m";
    static constexpr const char* GREEN = "\033[32m";
    static constexpr const char* BOLD = "\033[1m";
    static constexpr const char* UNDERLINE = "\033[4m";

    std::string level_color(Level level) const;
    std::string log_level(Level level) const;
    std::string log_level_formatted(Level level) const;

    std::string file_path(Level level, const Location& path) const;
  };
} // namespace phantom
