#include <LLVMCodeGen/Compiler.hpp>
#include <LLVMCodeGen/Visitor.hpp>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Metadata.h>
#include <llvm/IR/Module.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Passes/PassBuilder.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Path.h>
#include <llvm/Support/Program.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/TargetParser/Host.h>
#include <llvm/TargetParser/SubtargetFeature.h>
#include <llvm/Transforms/InstCombine/InstCombine.h>
#include <llvm/Transforms/Scalar/GVN.h>
#include <llvm/Transforms/Scalar/Reassociate.h>
#include <llvm/Transforms/Scalar/SimplifyCFG.h>

namespace phantom {
  namespace llvm_codegen {
    bool Compiler::compile() {
      auto context = std::make_shared<llvm::LLVMContext>();
      auto builder = std::make_shared<llvm::IRBuilder<>>(*context);
      auto module = std::make_shared<llvm::Module>(opts.out_file, *context);

      {
        // compiler Metadata
        llvm::NamedMDNode* identMD = module->getOrInsertNamedMetadata("llvm.ident");
        llvm::MDString* identStr = llvm::MDString::get(*context, compiler_metadata);
        llvm::MDNode* identNode = llvm::MDNode::get(*context, identStr);
        identMD->addOperand(identNode);
        module->setSourceFileName(opts.source_file);
      }

      std::string error;
      if (!generate_host_infos(module, error))
        Report("Cannot get HOST informations: " + error);

      Visitor visitor(context, builder, module);

      std::shared_ptr<llvm::FunctionPassManager> FPM = nullptr;
      std::shared_ptr<llvm::FunctionAnalysisManager> FAM = nullptr;
      std::shared_ptr<llvm::LoopAnalysisManager> LAM = nullptr;
      std::shared_ptr<llvm::ModuleAnalysisManager> MAM = nullptr;
      std::shared_ptr<llvm::CGSCCAnalysisManager> CGAM = nullptr;

      if (opts.opitimize) {
        FPM = std::make_shared<llvm::FunctionPassManager>();
        FAM = std::make_shared<llvm::FunctionAnalysisManager>();
        LAM = std::make_shared<llvm::LoopAnalysisManager>();
        MAM = std::make_shared<llvm::ModuleAnalysisManager>();
        CGAM = std::make_shared<llvm::CGSCCAnalysisManager>();

        FPM->addPass(llvm::InstCombinePass());
        FPM->addPass(llvm::ReassociatePass());
        FPM->addPass(llvm::GVNPass());
        FPM->addPass(llvm::SimplifyCFGPass());

        llvm::PassBuilder pass_builder;
        pass_builder.registerModuleAnalyses(*MAM);
        pass_builder.registerFunctionAnalyses(*FAM);
        pass_builder.crossRegisterProxies(*LAM, *FAM, *CGAM, *MAM);
      }

      visitor.set_optimizations(FPM, FAM, LAM, MAM, CGAM);

      for (auto& stt : ast)
        stt->accept(&visitor);

      std::error_code err_code;
      llvm::raw_fd_ostream file(opts.out_file, err_code, llvm::sys::fs::OF_None);

      if (err_code) {
        Report("Error opening file: " + err_code.message() + "\n", true);
        return false;
      }

      if (strcmp(opts.out_type.c_str(), "ir") == 0)
        return emit_ir(file, module);

      if (strcmp(opts.out_type.c_str(), "asm") == 0)
        return emit_asm(file, module);

      if (strcmp(opts.out_type.c_str(), "obj") == 0)
        return emit_obj(file, module);

      else
        return emit_exec(file, opts.out_file, module);
    }

    bool Compiler::generate_host_infos(std::shared_ptr<llvm::Module> module, std::string& error) {
      if (target_machine)
        return true;

      // initialize the HOST infos
      llvm::InitializeNativeTarget();
      llvm::InitializeNativeTargetAsmPrinter();
      llvm::InitializeNativeTargetAsmParser();

      std::string target_triplet = llvm::sys::getDefaultTargetTriple();
      const llvm::Target* target = llvm::TargetRegistry::lookupTarget(target_triplet, error);

      if (!target)
        return false;

      llvm::TargetOptions target_options;
      std::string cpu = llvm::sys::getHostCPUName().str();

      llvm::SubtargetFeatures features;
      llvm::StringMap<bool> host_features = llvm::sys::getHostCPUFeatures();

      for (const auto& feature : host_features)
        features.AddFeature(feature.first(), feature.second);

      this->target_machine = std::unique_ptr<llvm::TargetMachine>(
          target->createTargetMachine(
              target_triplet,
              cpu,
              features.getString(),
              target_options,
              llvm::Reloc::PIC_,
              llvm::CodeModel::Small));

      module->setTargetTriple(target_triplet);
      module->setDataLayout(target_machine->createDataLayout());

      return true;
    }

    bool Compiler::emit_ir(llvm::raw_fd_ostream& file, std::shared_ptr<llvm::Module> module) {
      module->print(file, nullptr);

      file.close();
      return true;
    }

    bool Compiler::emit_asm(llvm::raw_fd_ostream& file, std::shared_ptr<llvm::Module> module) {
      llvm::legacy::PassManager pass;
      llvm::CodeGenFileType filetype = llvm::CodeGenFileType::AssemblyFile;

      if (target_machine->addPassesToEmitFile(pass, file, nullptr, filetype)) {
        file.close();
        Report("TargetMachine can't emit assembly\n", true);
      }

      pass.run(*module);
      file.close();
      return true;
    }

    bool Compiler::emit_obj(llvm::raw_fd_ostream& file, std::shared_ptr<llvm::Module> module) {
      llvm::legacy::PassManager pass;
      llvm::CodeGenFileType filetype = llvm::CodeGenFileType::ObjectFile;

      if (target_machine->addPassesToEmitFile(pass, file, nullptr, filetype)) {
        file.close();
        Report("target_machine can't emit an object file", true);
      }

      pass.run(*module);
      file.close();
      return true;
    }

    bool Compiler::emit_exec(llvm::raw_fd_ostream& file, const std::string& file_path,
                             std::shared_ptr<llvm::Module> module) {
      llvm::SmallString<128> temp_obj_path;
      auto error_code = llvm::sys::fs::createTemporaryFile("temp_obj", "o", temp_obj_path);

      if (error_code) {
        file.close();
        Report("Cannot create temporary file: " + error_code.message() + "\n", true);
      }

      llvm::raw_fd_ostream obj_file(temp_obj_path, error_code, llvm::sys::fs::OF_None);
      if (error_code) {
        file.close();
        Report("Cannot open temp file: " + error_code.message() + "\n", true);
      }

      if (!emit_obj(obj_file, module))
        return false;

      std::vector<llvm::StringRef> args = {
          "clang",
          temp_obj_path.str(),
          "-o", file_path};

      auto clang_path = llvm::sys::findProgramByName("clang");

      if (!clang_path) {
        remove_file(temp_obj_path.str().str());
        file.close();
        Report("Could not find clang executable\n", true);
      }

      std::string error_msg;
      int result = llvm::sys::ExecuteAndWait(*clang_path, args, std::nullopt, {}, 0, 0, &error_msg);

      remove_file(temp_obj_path.str().str());

      if (result != 0) {
        file.close();
        Report("Linking failed: " + error_msg + "\n", true);
      }

      return true;
    }

    void Compiler::remove_file(const std::string& path) {
      auto error_code = llvm::sys::fs::remove(path);

      if (error_code)
        Report("Failed to remove temporary file " + path + ": " + error_code.message() + "\n", true);
    }
  } // namespace llvm_codegen
} // namespace phantom
