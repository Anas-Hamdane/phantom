#ifndef PHANTOM_INFO_HPP
#define PHANTOM_INFO_HPP

#include <string>
#include <vector>

namespace phantom {
  struct FileInfo {
    std::string path;
    std::string content;
    std::vector<std::string> content_lines;

    FileInfo(const std::string& file_path, const std::string& content, const std::vector<std::string>& content_lines)
      : path(file_path), content(content), content_lines(content_lines) {}
  };

  struct Location {
    static inline FileInfo file{"", "", {}};
    const size_t line;
    const size_t column;

    Location(const size_t line = 0, const size_t column = 0) : line(line), column(column) {}
  };
} // namespace phantom

#endif // PHANTOM_INFO_HPP
