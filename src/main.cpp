#include <Driver.hpp>
#include <Lexer.hpp>
#include <Parser.hpp>

// #include <codegen/Codegen.hpp>

using namespace phantom;

// Helper function to print indentation
void print_indent(int depth) {
    for (int i = 0; i < depth; i++) {
        printf("   ");
    }
    if (depth > 0) {
        printf("* ");
    }
}

void print_tokens(const std::vector<Token>& tokens) {
  for (const Token& token : tokens) {
    std::string type_str = Token::kind_to_string(token.kind);
    printf("TYPE: %-18s, FORM: \"%s\"\n", type_str.c_str(), token.form.c_str());
  }
}

FileInfo read_file(const std::string& file_path, Logger& logger) {
  std::string content = "";
  std::vector<std::string> content_lines;

  FILE* file = fopen(file_path.c_str(), "rb");
  if (!file)
    logger.log(Logger::Level::FATAL, "Failed to open file: " + file_path, true);

  if (fseek(file, 0, SEEK_END) != 0) {
    fclose(file);
    logger.log(Logger::Level::FATAL, "Failed to seek to end of file: " + file_path, true);
  }

  long file_size = ftell(file);
  if (file_size == -1) {
    fclose(file);
    logger.log(Logger::Level::FATAL, "Failed to get file size: " + file_path, true);
  }

  if (fseek(file, 0, SEEK_SET) != 0) {
    fclose(file);
    logger.log(Logger::Level::FATAL, "Failed to seek to beginning of file: " + file_path, true);
  }

  // Handle empty file
  if (file_size == 0) {
    fclose(file);
    return ::FileInfo(file_path, content, content_lines);
  }

  content.resize(file_size);
  size_t bytes_read = fread(content.data(), 1, file_size, file);
  fclose(file);

  if (bytes_read != static_cast<size_t>(file_size))
    logger.log(Logger::Level::FATAL, "Failed to read complete file: " + file_path, true);

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

  return FileInfo(file_path, content, content_lines);
}

int main(int argc, char* argv[]) {
  Logger logger;

  Driver driver(std::vector<std::string>(argv + 0, argv + argc), logger);
  Options opts = driver.parse_options();

  FileInfo file = read_file(opts.source_file, logger);
  Location::file = file;

  vec::Vec<Stmt> ast;
  {
    Lexer lexer(file.content, logger);
    auto tokens = lexer.lex();

    // print_tokens(tokens);

    // printf("\n-----------------------------------\n");

    Parser parser(tokens, logger);
    ast = parser.parse();

    // print_ast(ast, expr_area, stmt_area);
  }

  // codegen::Codegen gen(ast, expr_area, stmt_area);
  // const char* assm = gen.codegen();
  //
  // printf("%s", assm);

  return 0;
}
