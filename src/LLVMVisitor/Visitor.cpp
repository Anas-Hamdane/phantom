#include <LLVMVisitor/Visitor.hpp>
#include <global.hpp>

namespace phantom {
  // -------------constructor----------------
  Visitor::Visitor(const std::string& module_name) {
    context = std::make_unique<llvm::LLVMContext>();
    module = std::make_unique<llvm::Module>(module_name, *context);

    builder = std::make_unique<llvm::IRBuilder<>>(*context);
  }

  // -------------helper functions----------------
  llvm::Value* Visitor::cast(llvm::Value* src, llvm::Type* dst) {
    if (src->getType() == dst)
      return src;

    if (src->getType()->isIntegerTy() && dst->isIntegerTy())
      return builder->CreateIntCast(src, dst, true);

    else if (src->getType()->isIntegerTy() && dst->isFloatingPointTy())
      return builder->CreateSIToFP(src, dst);

    else if (src->getType()->isFloatingPointTy() && dst->isIntegerTy())
      return builder->CreateFPToSI(src, dst);

    else if (src->getType()->isFloatingPointTy() && dst->isFloatingPointTy())
      return builder->CreateFPCast(src, dst);

    else if (src->getType()->isPointerTy() && dst->isPointerTy())
      return builder->CreateBitCast(src, dst);

    else
      Report("Invalid type conversion\nExpected: \"" + get_string_type(dst) +
                 "\"\nGot: \"" + get_string_type(src->getType()) + "\"",
             true);

    return nullptr;
  }

  llvm::Value* Visitor::global_var_dec(VarDecStt* stt) {
    llvm::Constant* constant_init = nullptr;
    llvm::Type* variable_type = nullptr;
    llvm::Value* value = nullptr;

    std::string type = stt->variable.get_type();
    std::string name = stt->variable.get_name();

    // no way to get the type of the variable
    if (type.empty() && !stt->initializer)
      Report("Can not initialize type for \"" + name + "\"\n", true);

    // specified type
    if (!type.empty()) {
      variable_type = get_llvm_type(type);
      if (stt->initializer) {
        value = stt->initializer->accept(this);

        // cast the value to the variable type
        if (variable_type != value->getType())
          value = cast(value, variable_type);

        constant_init = llvm::dyn_cast<llvm::Constant>(value);
      }
    }
    // initializer is not a nullptr
    else {
      value = stt->initializer->accept(this);
      variable_type = value->getType();

      constant_init = llvm::dyn_cast<llvm::Constant>(value);
    }

    // declare the global variable
    llvm::GlobalVariable* global_variable = new llvm::GlobalVariable(
        *module,                            // module
        variable_type,                      // type
        false,                              // constant
        llvm::GlobalValue::ExternalLinkage, // linkage
        constant_init,                      // Constant
        name                           // name
    );

    stt->variable.set_alloca(global_variable);
    stt->variable.set_llvm_type(variable_type);
    stt->variable.set_global(true);

    named_values[name] = stt->variable;
    return global_variable;
  }

  llvm::Value* Visitor::local_var_dec(VarDecStt* stt) {
    llvm::Type* variable_type = nullptr;
    llvm::Value* value = nullptr;

    std::string type = stt->variable.get_type();
    std::string name = stt->variable.get_name();

    // no way to get the variable type
    if (type.empty() && !stt->initializer)
      Report("Can not initialize type for \"" + name + "\"\n", true);

    // specified type
    if (!type.empty()) {
      variable_type = get_llvm_type(type);
      if (stt->initializer) {
        value = stt->initializer->accept(this);

        if (value->getType() != variable_type)
          value = cast(value, variable_type);
      }
    }
    // initializer is not a nullptr
    else {
      value = stt->initializer->accept(this);
      variable_type = value->getType();
    }

    // create the alloca instruction
    llvm::AllocaInst* alloca = builder->CreateAlloca(variable_type, nullptr);

    stt->variable.set_alloca(alloca);
    stt->variable.set_llvm_type(variable_type);

    if (value)
      builder->CreateStore(value, alloca);

    // Store in symbol table for later lookups
    named_values[name] = stt->variable;

    // Return the alloca instruction (which is a Value*)
    return alloca;
  }

  void Visitor::print_representation() const {
    module->print(llvm::outs(), nullptr);
  }

  // -------------visit functions----------------
  // Integer Literal Expressions
  llvm::Value* Visitor::visit(IntLitExpr* expr) {
    llvm::Type* type = nullptr;

    if (expr->value >= INT_MIN_VAL && expr->value <= INT_MAX_VAL)
      type = llvm::Type::getInt32Ty(*context);

    else if (expr->value >= LONG_MIN_VAL && expr->value <= LONG_MAX_VAL)
      type = llvm::Type::getInt64Ty(*context);

    else
      Report("Value is too large to be represented in a data type\n", true);

    return llvm::ConstantInt::get(type, expr->value);
  }

  // Float Literal Expressions
  llvm::Value* Visitor::visit(FloatLitExpr* expr) {
    llvm::Type* type = nullptr;

    char last_character = expr->form.back();

    if (last_character == 'f' || last_character == 'F')
      type = llvm::Type::getFloatTy(*context);

    else if (last_character == 'd' || last_character == 'D')
      type = llvm::Type::getDoubleTy(*context);

    else if (expr->value >= FLOAT_MIN_VAL && expr->value <= FLOAT_MAX_VAL)
      type = llvm::Type::getFloatTy(*context);

    else if (expr->value >= DOUBLE_MIN_VAL && expr->value <= DOUBLE_MAX_VAL)
      type = llvm::Type::getDoubleTy(*context);

    else if (expr->value >= QUAD_MIN_VAL && expr->value <= QUAD_MAX_VAL)
      type = llvm::Type::getFP128Ty(*context);

    else
      Report("Value is too large to be represented in a data type\n", true);

    return llvm::ConstantFP::get(type, expr->value);
  }

  // Char Literal Expressions
  llvm::Value* Visitor::visit(CharLitExpr* expr) {
    return llvm::ConstantInt::get(llvm::Type::getInt8Ty(*context), expr->value);
  }

  // Bool Literal Expressions
  llvm::Value* Visitor::visit(BoolLitExpr* expr) {
    return llvm::ConstantInt::get(llvm::Type::getInt1Ty(*context), expr->value);
  }

  // String Literal Expressions
  llvm::Value* Visitor::visit(StrLitExpr* expr) {
    return builder->CreateGlobalString(expr->value);
  }

  // Identifier Expressions
  llvm::Value* Visitor::visit(IdentifierExpr* expr) {
    if (named_values.find(expr->name) == named_values.end())
      Report("Use of undeclared identifier \"" + expr->name + "\"\n", true);

    Variable variable = named_values[expr->name];
    llvm::Value* value = variable.get_alloca();

    // global variable
    if (variable.is_global()) {
      llvm::GlobalVariable* global_variable = llvm::cast<llvm::GlobalVariable>(value);
      llvm::Type* value_type = global_variable->getValueType();
      return builder->CreateLoad(value_type, value);
    }

    // locale variable
    else {
      llvm::AllocaInst* alloca_inst = llvm::cast<llvm::AllocaInst>(value);
      llvm::Type* allocated_type = alloca_inst->getAllocatedType();
      return builder->CreateLoad(allocated_type, value);
    }

    // others
    return value;
  }

  // Binary Operators Expressions
  llvm::Value* Visitor::visit(BinOpExpr* expr) {
    llvm::Value* right = expr->right->accept(this);

    if (!right)
      return nullptr;

    // special case: assignment expression
    if (expr->op == TokenType::EQUAL) {
      IdentifierExpr* idexpr = dynamic_cast<IdentifierExpr*>(expr->left.get());

      if (!idexpr)
        Report("Invalid identifier in assignment expression\n", true);

      llvm::Value* alloca = named_values[idexpr->name].get_alloca();

      if (!alloca)
        Report("Invalid value of an identifier in assignment expression\n", true);

      builder->CreateStore(right, alloca);
      return right;
    }

    llvm::Value* left = expr->left->accept(this);

    if (!left)
      return nullptr;

    switch (expr->op) {
      case TokenType::PLUS:
        return builder->CreateAdd(left, right, "addtmp");
      case TokenType::MINUS:
        return builder->CreateSub(left, right, "subtmp");
      case TokenType::STAR:
        return builder->CreateMul(left, right, "multmp");
      case TokenType::SLASH:
        return builder->CreateSDiv(left, right, "divtmp");
      default:
        Report("invalid binary operator", true);
    }

    return nullptr;
  }

  // Address Expressions (&x)
  llvm::Value* Visitor::visit(AddrExpr* expr) {
    llvm::Value* pointer = named_values[expr->variable].get_alloca();

    if (!pointer)
      Report("Use of undeclared identifier \"" + expr->variable + "\"\n", true);

    return pointer;
  }

  // Function Call Expressions
  llvm::Value* Visitor::visit(FnCallExpr* expr) {
    llvm::Function* calle_fn = module->getFunction(expr->name);

    if (!calle_fn)
      Report("Unknown function reference \"" + expr->name + "\"\n", true);

    if (calle_fn->arg_size() != expr->args.size())
      Report("Incorrect number of arguments for function \"" + expr->name + "\"\n", true);

    std::vector<llvm::Value*> args_values;
    args_values.reserve(expr->args.size());

    for (size_t i = 0; i < expr->args.size(); ++i) {
      args_values.push_back(expr->args[i]->accept(this));
      if (!args_values.back())
        return nullptr;
    }

    return builder->CreateCall(calle_fn, args_values, expr->name);
  }

  // Return Statements
  llvm::Value* Visitor::visit(ReturnStt* stt) {
    llvm::BasicBlock* currentBlock = builder->GetInsertBlock();

    if (!currentBlock || !currentBlock->getParent())
      Report("return outside of a function is not allowed\n", true);

    if (!stt->expr)
      return builder->CreateRetVoid();

    // return value
    llvm::Value* return_value = stt->expr->accept(this);

    if (!return_value)
      return nullptr;

    // current function return type
    llvm::Type* fn_return_type = currentBlock->getParent()->getReturnType();

    if (fn_return_type != return_value->getType())
      return_value = cast(return_value, fn_return_type);

    return builder->CreateRet(return_value);
  }

  // Expression Statements
  llvm::Value* Visitor::visit(ExprStt* stt) {
    return stt->expr->accept(this);
  }

  // Variable Declaration Statements
  llvm::Value* Visitor::visit(VarDecStt* stt) {
    if (named_values.find(stt->variable.get_name()) != named_values.end())
      Report("Variable redefinition for \"" + stt->variable.get_name() + "\"\n", true);

    if (!builder->GetInsertBlock())
      // global variable
      return global_var_dec(stt);

    // local variable
    return local_var_dec(stt);
  }

  // Function Declaration Statements
  llvm::Function* Visitor::visit(FnDecStt* stt) {
    // Convert parameter types
    std::vector<llvm::Type*> param_types;
    param_types.reserve(stt->params.size());

    for (const auto& param : stt->params) {
      llvm::Type* param_type = get_llvm_type(param.type);

      if (!param_type)
        Report("Unrecognized data type \"" + param.type + "\": \"" + param.name + "\"\n", true);

      param_types.push_back(param_type);
    }

    // Get return type
    llvm::Type* return_type = get_llvm_type(stt->type);
    if (!return_type)
      Report("Unrecognized return type \"" + stt->type + "\": \"" + stt->name + "\"\n", true);

    // Create function type
    llvm::FunctionType* fnType = llvm::FunctionType::get(
        return_type, // return type
        param_types, // prameters types
        false        // variable argument
    );

    // Create function
    llvm::Function* fn = llvm::Function::Create(
        fnType,
        llvm::Function::ExternalLinkage,
        0,
        stt->name,
        module.get());

    return fn;
  }

  // Function Definition Statements
  llvm::Function* Visitor::visit(FnDefStt* stt) {
    llvm::Function* fn = module->getFunction(stt->declaration->name);

    if (!fn)
      fn = stt->declaration->accept(this);

    if (!fn)
      Report("Can't declare function "
             "\"" +
                 stt->declaration->type + "\": \"" + stt->declaration->name + "\"\n",
             true);

    // has no body
    if (!fn->empty())
      Report("Function cannot be redefined \"" +
                 stt->declaration->type + "\": \"" + stt->declaration->name + "\"\n",
             true);

    // Create basic block for function body
    llvm::BasicBlock* BB = llvm::BasicBlock::Create(*context, "entry", fn);
    builder->SetInsertPoint(BB);

    /*
     * NOTE: for nested scopes, save the named_values as "old_named_values"
     * add the arguments of the functions to the named_values, generate the code
     * for function body, restore the "old_named_values" in named_values.
     */
    auto old_named_values = named_values;

    // arguments handling
    size_t idx = 0;
    for (auto& arg : fn->args()) {
      llvm::AllocaInst* alloca = builder->CreateAlloca(arg.getType());
      builder->CreateStore(&arg, alloca);

      std::string name = stt->declaration->params[idx].name;
      named_values[name] = Variable(name, stt->declaration->params[idx].type, alloca, alloca->getType());
      ++idx;
    }

    // Generate code for function body
    // note: each statement has it's own error handling
    // so probably, if an error occurs, the compiler will
    // shut down and we won't came back here
    for (auto& stmt : stt->body) {
      if (!stmt->accept(this))
        return nullptr;
    }

    // adjust return value stuff
    llvm::BasicBlock* currentBlock = builder->GetInsertBlock();
    if (currentBlock && !currentBlock->getTerminator()) {
      llvm::Type* ret_value = fn->getReturnType();

      if (ret_value->isVoidTy())
        builder->CreateRetVoid();

      else if (ret_value->isIntegerTy())
        // default return value if not specified:
        // 0 for i1, i8, i32, i64
        builder->CreateRet(llvm::ConstantInt::get(ret_value, 0));

      else if (ret_value->isFloatTy() || ret_value->isDoubleTy())
        builder->CreateRet(llvm::ConstantFP::get(ret_value, 0.0));

      /* TODO: add pointer handling*/

      else
        builder->CreateUnreachable();
    }

    // clear this scope's variables
    named_values = old_named_values;
    // back to the previous scope
    builder->ClearInsertionPoint();

    // Verify the function
    if (llvm::verifyFunction(*fn, &llvm::errs())) {
      Report("Function verification failed for \"" +
             stt->declaration->type + "\": \"" + stt->declaration->name + "\"\n");
      fn->eraseFromParent();
      return nullptr;
    }

    return fn;
  }
} // namespace phantom
