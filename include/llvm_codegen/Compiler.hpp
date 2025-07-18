#ifndef PHANTOM_COMPILER_HPP
#define PHANTOM_COMPILER_HPP

#include <Logger.hpp>
#include <driver/Options.hpp>
#include <vector>
#include <memory>

#include "llvm/Target/TargetMachine.h"

namespace phantom {
  class Statement;
  class Options;
  namespace llvm_codegen {
    class Compiler {
      std::unique_ptr<llvm::TargetMachine> target_machine = nullptr;
      const std::vector<std::unique_ptr<Statement>>& ast;
      const Options& opts;
      const Logger& logger;

      bool generate_host_infos(std::shared_ptr<llvm::Module> module, std::string& error);

      bool emit_ir(llvm::raw_fd_ostream& file, std::shared_ptr<llvm::Module> module);
      bool emit_asm(llvm::raw_fd_ostream& file, std::shared_ptr<llvm::Module> module);
      bool emit_obj(llvm::raw_fd_ostream& file, std::shared_ptr<llvm::Module> module);
      bool emit_exec(llvm::raw_fd_ostream& file, const std::string& path, std::shared_ptr<llvm::Module> module);

      void remove_file(const std::string& path);

  public:
      Compiler(const std::vector<std::unique_ptr<Statement>>& ast, const Options& opts, const Logger& logger)
          : ast(ast), opts(opts), logger(logger) {}

      bool compile();
    };
  } // namespace llvm_codegen
} // namespace phantom

#endif // !PHANTOM_COMPILER_HPP
