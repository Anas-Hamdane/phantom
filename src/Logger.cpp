#include <Logger.hpp>
#include <info.hpp>

namespace phantom {
  std::string Logger::level_color(Level level) const {
    switch (level) {
      case Level::FATAL:
      case Level::ERROR:
        return this->RED;
      case Level::WARNING:
        return this->YELLOW;
      case Level::INFO:
        return this->BLUE;
      case Level::DEBUG:
        return this->GREEN;
    }

    return "";
  }

  std::string Logger::log_level(Level level) const {
    if (colored)
      return log_level_formatted(level);

    switch (level) {
      case Level::DEBUG:
        return "DEBUG";
      case Level::INFO:
        return "INFO";
      case Level::WARNING:
        return "WARNING";
      case Level::ERROR:
        return "ERROR";
      case Level::FATAL:
        return "FATAL";
    }

    return "";
  }

  std::string Logger::log_level_formatted(Level level) const {
    switch (level) {
      case Level::DEBUG:
        return std::string(this->GREEN) + "DEBUG" + this->RESET;
      case Level::INFO:
        return std::string(this->BLUE) + "INFO" + this->RESET;
      case Level::WARNING:
        return std::string(this->YELLOW) + "WARNING" + this->RESET;
      case Level::ERROR:
        return std::string(this->RED) + "ERROR" + this->RESET;
      case Level::FATAL:
        return std::string(this->RED) + "FATAL" + this->RESET;
    }

    return "";
  }

  std::string Logger::file_path(Level level, const Location& location) const {
    std::string result;
    if (colored)
      result += std::string(this->UNDERLINE) + level_color(level);

    result += location.file.path;

    if (location.line != 0) result += ':' + std::to_string(location.line);
    if (location.column != 0) result += ':' + std::to_string(location.column);

    if (colored)
      result += std::string(this->RESET);

    result += '\n';

    return result;
  }

  void Logger::log(Level level, const std::string& message, const Location& location, const bool exit_, FILE* stream) const {
    std::string complete_message = "[" + log_level(level) + "] -> " + file_path(level, location);

    std::string line = std::to_string(location.line);

    if (location.line != 0 && ((location.line - 1) < location.file.content.size())) {
      complete_message += std::string(line.length() + 1, ' ') + "|\n";
      complete_message += line + " | " + location.file.content_lines[location.line - 1] + "\n";
      complete_message += std::string(line.length() + 1, ' ') + "| ";

      if (location.column != 0)
        complete_message += std::string(location.column - 1, ' ') + "^";

      complete_message += '\n';
    }

    complete_message += colored ? (std::string(this->BOLD) + message) : message;
    complete_message += '\n';

    if (stream)
      fwrite(complete_message.c_str(), 1, complete_message.length(), stream);

    if (exit_)
      exit(level);
  }

  void Logger::log(Level level, const std::string& message, const bool exit_, FILE* stream) const {
    std::string complete_message = "[" + log_level(level) + "]\n";

    if (colored)
      complete_message += this->BOLD + message + this->RESET;
    else
      complete_message += message;

    complete_message += '\n';

    if (stream)
      fwrite(complete_message.c_str(), 1, complete_message.length(), stream);

    if (exit_)
      exit(level);
  }
} // namespace phantom
