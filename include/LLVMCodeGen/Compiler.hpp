#ifndef PHANTOM_COMPILER_HPP
#define PHANTOM_COMPILER_HPP

#include <Options.hpp>
#include <Parser/Statements.hpp>
#include <llvm/Target/TargetMachine.h>

namespace phantom {
  namespace llvm_codegen {
    class Compiler {
      std::vector<std::unique_ptr<Statement>> ast;
      Options opts;
      std::unique_ptr<llvm::TargetMachine> target_machine = nullptr;

      bool generate_host_infos(std::shared_ptr<llvm::Module> module, std::string& error);

      bool emit_ir(llvm::raw_fd_ostream& file, std::shared_ptr<llvm::Module> module);
      bool emit_asm(llvm::raw_fd_ostream& file, std::shared_ptr<llvm::Module> module);
      bool emit_obj(llvm::raw_fd_ostream& file, std::shared_ptr<llvm::Module> module);
      bool emit_exec(llvm::raw_fd_ostream& file, const std::string& path, std::shared_ptr<llvm::Module> module);

      void remove_file(const std::string& path);

  public:
      Compiler(std::vector<std::unique_ptr<Statement>> ast, const Options& opts) : ast(std::move(ast)), opts(std::move(opts)) {}
      bool compile();
    };
  } // namespace llvm_codegen
} // namespace phantom

#endif // !PHANTOM_COMPILER_HPP
