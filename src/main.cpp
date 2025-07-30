#include "Driver.hpp"
#include "Lexer.hpp"
#include "Parser.hpp"
#include "irgen/Gen.hpp"

// #include <codegen/Codegen.hpp>

using namespace phantom;

void print_tokens(const std::vector<Token>& tokens) {
  for (const Token& token : tokens) {
    std::string type_str = Token::kind_to_string(token.kind);
    printf("TYPE: %-18s, FORM: \"%s\"\n", type_str.c_str(), token.form.c_str());
  }
}

const char* resolve_type(Type ty) {
  switch (ty) {
    case Type::Void:
      return "void";
    case Type::Bool:
      return "<char/bool>";
    case Type::Short:
      return "short";
    case Type::Int:
      return "int";
    case Type::Long:
      return "long";
    case Type::Half:
      return "half";
    case Type::Float:
      return "float";
    case Type::Double:
      return "double";
  }
}

const char* resolve_reg(ir::Register reg) {
  char* s;
  asprintf(&s, "%%%u", reg.id);

  return s;
}

const char* resolve_const(ir::Constant con) {
  char* s;

  switch (con.type) {
    case Type::Char:
    case Type::Short:
    case Type::Int:
    case Type::Long:
      asprintf(&s, "%lu", con.value.int_val);
      break;
    default:
      asprintf(&s, "%lf", con.value.float_val);
      break;
  }

  return s;
}

const char* resolve_value(ir::Value v) {
  if (v.kind == ir::ValueKind::Register)
    return resolve_reg(v.value.reg);

  return resolve_const(v.value.con);
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
  switch (inst.kind) {
    case ir::InstrKind::Alloca:
      printf("  alloca %s, %s\n", resolve_type(inst.inst.alloca.type), resolve_reg(inst.inst.alloca.reg));
      break;
    case ir::InstrKind::Load:
      printf("  load %s, %s\n", resolve_reg(inst.inst.load.src), resolve_reg(inst.inst.load.dst));
      break;
    case ir::InstrKind::Store:
      printf("  store %s, %s\n", resolve_value(inst.inst.store.src), resolve_reg(inst.inst.store.dst));
      break;
    case ir::InstrKind::BinOp:
      printf("  %s %s, %s -> %s\n", binop_op(inst.inst.binop.op), resolve_value(inst.inst.binop.lhs), resolve_value(inst.inst.binop.rhs), resolve_reg(inst.inst.binop.dst));
      break;
    case ir::InstrKind::UnOp:
      printf("  %s %s -> %s\n", unop_op(inst.inst.unop.op), resolve_value(inst.inst.unop.operand), resolve_reg(inst.inst.unop.dst));
      break;
  }
}

void print_block(ir::BasicBlock block) {
  printf("%u:\n", block.id);

  for (auto inst : block.insts) {
    print_instruction(inst);
  }
}

void print_program(ir::Program& program) {
  for (auto fn : program.funcs) {
    printf("%s %s() -> %s {\n", fn.defined ? "define" : fn.externed ? "extern"
                                                                    : "declare",
           fn.name, resolve_type(fn.return_type));
    for (auto block : fn.blocks) {
      print_block(block);
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

  utils::Vec<Stmt> ast;
  {
    Lexer lexer(file.content, logger);
    auto tokens = lexer.lex();

    // print_tokens(tokens);

    // printf("\n-----------------------------------\n");

    Parser parser(tokens, logger);
    ast = parser.parse();

    // print_ast(ast, expr_area, stmt_area);
  }

  ir::Gen irgen(ast);
  ir::Program prog = irgen.gen();

  print_program(prog);

  return 0;
}
