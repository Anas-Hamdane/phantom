#include "Driver.hpp"
#include "Lexer.hpp"
#include "ast/Parser.hpp"
#include "irgen/Gen.hpp"
#include <cstring>
#include "codegen/Codegen.hpp"

using namespace phantom;

void print_tokens(const std::vector<Token>& tokens) {
  for (const Token& token : tokens) {
    std::string type_str = Token::kind_to_string(token.kind);
    printf("TYPE: %-18s, FORM: \"%s\"\n", type_str.c_str(), token.form.c_str());
  }
}

const char* resolve_type(Type ty) {
  std::string s;
  // clang-format off
  switch (ty.kind) {
    case Type::Kind::Void:   s += "void"; break;
    case Type::Kind::Int:    s += "i"; break;
    case Type::Kind::UnsInt: s += "u"; break;
    case Type::Kind::FP:     s += "f"; break;
  }
  // clang-format on

  if (ty.bitwidth != 0)
    s += std::to_string(ty.bitwidth);

  const char* s_leg = strdup(s.c_str());
  return s_leg;
}

const char* resolve_reg(ir::Register reg) {
  char* s;
  asprintf(&s, "%%%u", reg.id);

  return s;
}
const char* resolve_const(ir::Constant con) {
  char* s;

  switch (con.value.index()) {
    case 0:
      asprintf(&s, "%lu", std::get<0>(con.value));
      break;
    case 1:
      asprintf(&s, "%lf", std::get<1>(con.value));
      break;
  }

  return s;
}
const char* resolve_value(ir::Value v) {
  switch (v.index()) {
    case 0:
      return resolve_reg(std::get<0>(v));
    case 1:
      return resolve_const(std::get<1>(v));
  }

  std::abort();
}

const char* binop_op(ir::BinOp::Op op) {
  switch (op) {
    case ir::BinOp::Op::Add:
      return "add";
    case ir::BinOp::Op::Sub:
      return "sub";
    case ir::BinOp::Op::Mul:
      return "mul";
    case ir::BinOp::Op::Div:
      return "div";
  }
}
const char* unop_op(ir::UnOp::Op op) {
  switch (op) {
    case ir::UnOp::Op::Neg:
      return "neg";
    case ir::UnOp::Op::Not:
      return "not";
  }
}

void print_instruction(ir::Instruction inst) {
  // clang-format off
  switch (inst.index()) {
    case 0: printf("  alloca %s, %s\n", resolve_type(std::get<0>(inst).type), resolve_reg(std::get<0>(inst).reg)); break;
    case 1: printf("  store %s, %s\n", resolve_value(std::get<1>(inst).src), resolve_reg(std::get<1>(inst).dst)); break;
    case 2: printf("  %s = %s %s, %s \n", resolve_reg(std::get<2>(inst).dst), binop_op(std::get<2>(inst).op), resolve_value(std::get<2>(inst).lhs), resolve_value(std::get<2>(inst).rhs)); break;
    case 3: printf("  %s = %s %s\n", resolve_reg(std::get<3>(inst).dst), unop_op(std::get<3>(inst).op), resolve_value(std::get<3>(inst).operand)); break;
  }
  // clang-format on
}
void print_terminator(ir::Terminator t) {
  switch (t.index()) {
    case 0:
      printf("  ret %s\n", resolve_value(std::get<0>(t).value));
  }
}
void print_program(ir::Program& program) {
  for (auto fn : program.funcs) {
    printf("%s %s(", fn.defined ? "define" : "declare", fn.name.c_str());

    size_t params_size = fn.params.size();
    for (size_t i = 0; i < params_size; ++i) {
      auto param = fn.params.at(i);
      printf("%s: %s", resolve_reg(param), resolve_type(param.type));

      if (i + 1 < params_size)
        printf(", ");
    }

    printf(") -> %s {\n", resolve_type(fn.return_type));

    for (auto inst : fn.body) {
      print_instruction(inst);
    }

    if (fn.terminated) {
      print_terminator(fn.terminator);
    }

    printf("}\n");
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

  std::vector<std::unique_ptr<ast::Stmt>> ast;
  {
    Lexer lexer(file.content, logger);
    auto tokens = lexer.lex();

    // print_tokens(tokens);

    // printf("\n-----------------------------------\n");

    ast::Parser parser(tokens, logger);
    ast = parser.parse();

    // print_ast(ast, expr_area, stmt_area);
  }

  ir::Gen irgen(ast);
  ir::Program prog = irgen.gen();

  // print_program(prog);

  codegen::Gen codegen(prog);
  const char* assembly = codegen.gen();

  printf("%s", assembly);

  return 0;
}
