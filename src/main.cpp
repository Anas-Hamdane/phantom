#define LLVM_BACKEND

#include <Lexer.hpp>
#include <Logger.hpp>
#include <Parser.hpp>
#include <Token.hpp>
#include <driver/Driver.hpp>
#include <driver/Options.hpp>

#include <common.hpp>
#include <info.hpp>
#include <llvm_codegen/Compiler.hpp>

#include <iostream>

void print_tokens(const std::vector<phantom::Token>& tokens) {
  for (const phantom::Token& token : tokens) {
    std::string type_str = phantom::Token::get_token_type(token.type);
    printf("TYPE: %-18s, FORM: \"%s\"\n", type_str.c_str(), token.form.c_str());
  }
}

phantom::FileInfo read_file(const std::string& file_path, phantom::Logger& logger) {
  std::string content = "";
  std::vector<std::string> content_lines;

  FILE* file = fopen(file_path.c_str(), "rb");
  if (!file)
    logger.log(phantom::Logger::Level::FATAL, "Failed to open file: " + file_path, true);

  if (fseek(file, 0, SEEK_END) != 0) {
    fclose(file);
    logger.log(phantom::Logger::Level::FATAL, "Failed to seek to end of file: " + file_path, true);
  }

  long file_size = ftell(file);
  if (file_size == -1) {
    fclose(file);
    logger.log(phantom::Logger::Level::FATAL, "Failed to get file size: " + file_path, true);
  }

  if (fseek(file, 0, SEEK_SET) != 0) {
    fclose(file);
    logger.log(phantom::Logger::Level::FATAL, "Failed to seek to beginning of file: " + file_path, true);
  }

  // Handle empty file
  if (file_size == 0) {
    fclose(file);
    return phantom::FileInfo(file_path, content, content_lines);
  }

  content.resize(file_size);
  size_t bytes_read = fread(content.data(), 1, file_size, file);
  fclose(file);

  if (bytes_read != static_cast<size_t>(file_size))
    logger.log(phantom::Logger::Level::FATAL, "Failed to read complete file: " + file_path, true);

  content_lines.reserve((file_size / 60) + 1);

  const char* start = content.c_str();
  const char* end = start + file_size;
  const char* line_start = start;

  for (const char* p = start; p < end; ++p) {
    if (*p == '\n') {
      content_lines.emplace_back(line_start, p - line_start);
      line_start = p + 1;
    }
  }

  if (line_start < end)
    content_lines.emplace_back(line_start, end - line_start);

  return phantom::FileInfo(file_path, content, content_lines);
}

int main(int argc, char* argv[]) {
  using namespace phantom;

  Logger logger;

  Driver driver(std::vector<std::string>(argv + 0, argv + argc), logger);
  Options opts = driver.parse_options();

  FileInfo file = read_file(opts.source_file, logger);
  phantom::Location::file = file;

  std::vector<std::unique_ptr<Statement>> ast;

  {
    Lexer lexer(file.content, logger);
    auto tokens = lexer.lex();

    if (opts.print == "tokens")
      print_tokens(tokens);

    Parser parser(tokens, logger);
    ast = parser.parse();
  }

  llvm_codegen::Compiler compiler(ast, opts, logger);

  if (!compiler.compile())
    logger.log(Logger::Level::FATAL, "Compilation failed.", true);

  std::cout << "Compilation terminated.\n";
  return 0;
}
