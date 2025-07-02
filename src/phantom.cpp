#include <Lexer/Lexer.hpp>
#include <Parser/Parser.hpp>
#include <LLVMVisitor/Visitor.hpp>

#include <fstream>
#include <sstream>

int main (int argc, char *argv[]) {
  using namespace phantom;

  if (argc < 2) {
    std::cerr << "Not enough args\n";
    return 1;
  }

  std::string path = argv[1];

  std::fstream file(argv[1]);
  std::stringstream buff;

  buff << file.rdbuf();

  Lexer lexer(buff.str());
  auto tokens = lexer.lex();

  Parser parser(tokens);
  auto stts = parser.parse();

  Visitor visitor(path.substr(path.find_last_of("/\\") + 1));

  for (auto& stt : stts)
    stt->accept(&visitor);

  visitor.print_representation();
  return 0;
}
