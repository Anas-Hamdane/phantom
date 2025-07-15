#include <Driver.hpp>
#include <Lexer/Lexer.hpp>
#include <Parser/Parser.hpp>

#include <LLVMCodeGen/Compiler.hpp>

#include <fstream>
#include <sstream>

int main(int argc, char* argv[]) {
  using namespace phantom;

  Driver driver(std::vector<std::string>(argv + 1, argv + argc));
  Options opts = driver.parse_options();

  std::string content;

  {
    std::fstream file(opts.source_file);
    std::stringstream buff;

    buff << file.rdbuf();
    content = buff.str();
  }

  std::vector<std::unique_ptr<Statement>> ast;

  {
    Lexer lexer(content);
    auto tokens = lexer.lex();

    Parser parser(tokens);
    ast = parser.parse();
  }

  llvm_codegen::Compiler compiler(std::move(ast), opts);
  
  if (!compiler.compile())
    Report("Compilation failed.\n", true);

  std::cout << "Compilation terminated.\n";
  return 0;
}
