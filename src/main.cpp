#include <Driver.hpp>
#include <Lexer.hpp>
#include <Parser.hpp>

#include <codegen/Codegen.hpp>

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

// Forward declaration for mutual recursion
void print_expr(ExprRef expr_ref, const ExprArea& expr_area, int depth);

// Print a statement node
void print_stmt(StmtRef stmt_ref, const ExprArea& expr_area, const StmtArea& stmt_area, int depth) {
    // Check for invalid reference (assuming 0 is invalid)
    if (stmt_ref == 0) return;
    const Stmt& stmt = stmt_area.get(stmt_ref);
    
    print_indent(depth);
    
    switch (stmt.kind) {
        case StmtKind::Invalid:
            printf("Invalid\n");
            break;
            
        case StmtKind::Return:
            printf("Return\n");
            if (stmt.data.ret.expr != 0) {
                print_expr(stmt.data.ret.expr, expr_area, depth + 1);
            }
            break;
            
        case StmtKind::Expmt:
            printf("Expmt\n");
            if (stmt.data.expmt.expr != 0) {
                print_expr(stmt.data.expmt.expr, expr_area, depth + 1);
            }
            break;
            
        case StmtKind::FnDecl:
            printf("FnDecl: %s\n", stmt.data.fn_decl.name);
            if (stmt.data.fn_decl.type != 0) {
                print_expr(stmt.data.fn_decl.type, expr_area, depth + 1);
            }
            // Print parameters if they exist
            if (stmt.data.fn_decl.params != nullptr) {
                for (size_t i = 0; i < stmt.data.fn_decl.params_len; i++) {
                    if (stmt.data.fn_decl.params[i] != 0) {
                        print_expr(stmt.data.fn_decl.params[i], expr_area, depth + 1);
                    }
                }
            }
            break;
            
        case StmtKind::FnDef:
            printf("FnDef\n");
            if (stmt.data.fn_def.declaration != 0) {
                print_stmt(stmt.data.fn_def.declaration, expr_area, stmt_area, depth + 1);
            }
            // Print function body statements if they exist
            if (stmt.data.fn_def.body != nullptr) {
                for (size_t i = 0; i < stmt.data.fn_def.body_len; i++) {
                    if (stmt.data.fn_def.body[i] != 0) {
                        print_stmt(stmt.data.fn_def.body[i], expr_area, stmt_area, depth + 1);
                    }
                }
            }
            break;
    }
}

// Print an expression node
void print_expr(ExprRef expr_ref, const ExprArea& expr_area, int depth) {
    // Check for invalid reference (assuming 0 is invalid)
    if (expr_ref == 0) return;
    const Expr& expr = expr_area.get(expr_ref);
    
    print_indent(depth);
    
    switch (expr.kind) {
        case ExprKind::Invalid:
            printf("Invalid\n");
            break;
            
        case ExprKind::DataType:
            printf("DataType: %s\n", expr.data.data_type.type);
            if (expr.data.data_type.length != 0) {
                print_expr(expr.data.data_type.length, expr_area, depth + 1);
            }
            break;
            
        case ExprKind::IntLit:
            printf("IntLit: %lu\n", expr.data.int_lit.value);
            break;
            
        case ExprKind::FloatLit:
            printf("FloatLit: %Lf\n", expr.data.float_lit.value);
            break;
            
        case ExprKind::CharLit:
            printf("CharLit: '%c'\n", expr.data.char_lit.value);
            break;
            
        case ExprKind::BoolLit:
            printf("BoolLit: %s\n", expr.data.bool_lit.value ? "true" : "false");
            break;
            
        case ExprKind::StrLit:
            printf("StrLit: \"%s\"\n", expr.data.str_lit.value);
            break;
            
        case ExprKind::ArrLit:
            printf("ArrLit\n");
            if (expr.data.arr_lit.elems != nullptr) {
                for (size_t i = 0; i < expr.data.arr_lit.len; i++) {
                    if (expr.data.arr_lit.elems[i] != 0) {
                        print_expr(expr.data.arr_lit.elems[i], expr_area, depth + 1);
                    }
                }
            }
            break;
            
        case ExprKind::Identifier:
            printf("Identifier: %s\n", expr.data.ide.name);
            break;
            
        case ExprKind::BinOp:
            printf("BinOp\n");
            if (expr.data.binop.left != 0) {
                print_expr(expr.data.binop.left, expr_area, depth + 1);
            }
            print_indent(depth + 1);
            printf("Op: %d\n", static_cast<int>(expr.data.binop.op));
            if (expr.data.binop.right != 0) {
                print_expr(expr.data.binop.right, expr_area, depth + 1);
            }
            break;
            
        case ExprKind::UnOp:
            printf("UnOp\n");
            print_indent(depth + 1);
            printf("Op: %d\n", static_cast<int>(expr.data.unop.op));
            if (expr.data.unop.expr != 0) {
                print_expr(expr.data.unop.expr, expr_area, depth + 1);
            }
            break;
            
        case ExprKind::VarDecl:
            printf("VarDecl: %s\n", expr_area.get(expr.data.var_decl.ide).data.ide.name);
            if (expr.data.var_decl.type != 0) {
                print_expr(expr.data.var_decl.type, expr_area, depth + 1);
            }
            if (expr.data.var_decl.value != 0) {
                print_expr(expr.data.var_decl.value, expr_area, depth + 1);
            }
            break;
            
        case ExprKind::FnCall:
            printf("FnCall: %s\n", expr.data.fn_call.name);
            if (expr.data.fn_call.args != nullptr) {
                for (size_t i = 0; i < expr.data.fn_call.len; i++) {
                    if (expr.data.fn_call.args[i] != 0) {
                        print_expr(expr.data.fn_call.args[i], expr_area, depth + 1);
                    }
                }
            }
            break;
    }
}

// Main print_ast function
void print_ast(const std::vector<StmtRef>& ast, const ExprArea& expr_area, const StmtArea& stmt_area) {
    for (const StmtRef& stmt_ref : ast) {
        if (stmt_ref != 0) {
            print_stmt(stmt_ref, expr_area, stmt_area, 0);
        }
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

  std::vector<StmtRef> ast;
  ExprArea expr_area;
  StmtArea stmt_area;

  {
    Lexer lexer(file.content, logger);
    auto tokens = lexer.lex();

    // print_tokens(tokens);

    // printf("\n-----------------------------------\n");

    Parser parser(tokens, logger, expr_area, stmt_area);
    ast = parser.parse();

    // print_ast(ast, expr_area, stmt_area);
  }

  codegen::Codegen gen(ast, expr_area, stmt_area);
  const char* assm = gen.codegen();

  printf("%s", assm);

  return 0;
}
