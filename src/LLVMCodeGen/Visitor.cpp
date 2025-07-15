#include <LLVMCodeGen/Visitor.hpp>

namespace phantom {
  Visitor::Visitor(std::shared_ptr<llvm::LLVMContext> context,
                   std::shared_ptr<llvm::IRBuilder<>> builder,
                   std::shared_ptr<llvm::Module> module)
      : context(context), builder(builder), module(module),
        operation(std::make_unique<Operation>(builder)) {}

  void Visitor::set_optimizations(std::shared_ptr<llvm::FunctionPassManager> FPM,
                                  std::shared_ptr<llvm::FunctionAnalysisManager> FAM,
                                  std::shared_ptr<llvm::LoopAnalysisManager> LAM,
                                  std::shared_ptr<llvm::ModuleAnalysisManager> MAM,
                                  std::shared_ptr<llvm::CGSCCAnalysisManager> CGAM) {

    this->FPM = FPM;
    this->FAM = FAM;
    this->LAM = LAM;
    this->MAM = MAM;
    this->CGAM = CGAM;
  }

  llvm::Value* Visitor::cast(llvm::Value* src, llvm::Type* dst, std::string error_msg) {
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
      Report(error_msg, true);

    return nullptr;
  }

  ExpressionInfo Visitor::global_var_dec(VarDecStt* stt) {
    llvm::Constant* constant_init = nullptr;
    llvm::Type* variable_type = nullptr;
    llvm::Value* value = nullptr;

    std::string name = stt->variable.name;

    if (!stt->initializer)
      Report("Can not initialize type for variable \"" + name + "\"\n", true);

    // initializer available
    ExpressionInfo init = stt->initializer->rvalue(this);

    if (!init.value || !init.type)
      Report("Internal compiler error: not enough informations about variable\"" + name + "\"\n", true);

    value = init.value;
    variable_type = init.type;

    if (variable_type->isPointerTy() && init.variable)
      stt->variable.ptr_to_variable = init.variable;

    else if (variable_type != value->getType()) {
      std::string cast_error = "Invalid operation: casting \"" +
                               get_string_type(value->getType()) + "\" to \"" +
                               get_string_type(variable_type) + "\", for variable \"" + name + "\"\n";

      value = cast(value, variable_type, cast_error);
    }

    constant_init = llvm::dyn_cast<llvm::Constant>(value);

    // declare the global variable
    llvm::GlobalVariable* global_variable = new llvm::GlobalVariable(
        *module,                            // module
        variable_type,                      // type
        false,                              // constant
        llvm::GlobalValue::ExternalLinkage, // linkage
        constant_init,                      // Constant
        name                                // name
    );

    stt->variable.value = global_variable;
    stt->variable.type = variable_type;
    stt->variable.global = true;

    named_variables[name] = stt->variable;
    return {global_variable, variable_type, &named_variables[name]};
  }

  ExpressionInfo Visitor::local_var_dec(VarDecStt* stt) {
    llvm::Type* variable_type = nullptr;
    llvm::Value* value = nullptr;

    std::string name = stt->variable.name;

    if (!stt->initializer)
      Report("Can not initialize type for variable \"" + name + "\"\n", true);

    // initializer available
    ExpressionInfo init = stt->initializer->rvalue(this);

    if (!init.value || !init.type)
      Report("Internal compiler error: not enough informations about variable\"" + name + "\"\n", true);

    value = init.value;
    variable_type = init.type;

    if (variable_type->isPointerTy() && init.variable)
      stt->variable.ptr_to_variable = init.variable;

    else if (variable_type != value->getType()) {
      std::string cast_error = "Invalid operation: casting \"" +
                               get_string_type(value->getType()) + "\" to \"" +
                               get_string_type(variable_type) + "\", for variable \"" + name + "\"\n";

      value = cast(value, variable_type, cast_error);
    }

    // create the alloca instruction
    llvm::AllocaInst* alloca = builder->CreateAlloca(variable_type, nullptr);

    if (value)
      builder->CreateStore(value, alloca);

    stt->variable.value = alloca;
    stt->variable.type = variable_type;

    // Store in symbol table for later lookups
    named_variables[name] = stt->variable;

    // Return the alloca instruction (which is a Value*)
    return {alloca, variable_type, &named_variables[name]};
  }

  ExpressionInfo Visitor::rvalue(DataTypeExpr* expr) {
    llvm::Value* value = nullptr;
    llvm::Type* type = get_llvm_type(expr->form);

    if (expr->value)
      value = expr->value->rvalue(this).value;

    if (value)
      return {value, type};

    if (type->isIntegerTy())
      value = llvm::ConstantInt::get(type, 0);

    else if (type->isFloatingPointTy())
      value = llvm::ConstantFP::get(type, 0.0);

    else if (type->isPointerTy())
      value = llvm::Constant::getNullValue(type);

    else
      Report("Unknown variable type: \"" + expr->form + "\"\n", true);

    return {value, type};
  }
  ExpressionInfo Visitor::lvalue(DataTypeExpr* expr) { return {}; }

  ExpressionInfo Visitor::rvalue(IntLitExpr* expr) {
    llvm::Type* type = nullptr;

    if (expr->value >= INT_MIN_VAL && expr->value <= INT_MAX_VAL)
      type = llvm::Type::getInt32Ty(*context);

    else if (expr->value >= LONG_MIN_VAL && expr->value <= LONG_MAX_VAL)
      type = llvm::Type::getInt64Ty(*context);

    else
      Report("Value is too large to be represented in a data type\n", true);

    return {llvm::ConstantInt::get(type, expr->value), type};
  }
  ExpressionInfo Visitor::lvalue(IntLitExpr* expr) { return {}; }

  ExpressionInfo Visitor::rvalue(FloatLitExpr* expr) {
    llvm::Type* type = nullptr;

    if (const char last_character = expr->form.back(); last_character == 'f' || last_character == 'F')
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

    return {llvm::ConstantFP::get(type, expr->value), type};
  }
  ExpressionInfo Visitor::lvalue(FloatLitExpr* expr) { return {}; }

  ExpressionInfo Visitor::rvalue(CharLitExpr* expr) {
    llvm::Type* type = llvm::Type::getInt8Ty(*context);
    return {llvm::ConstantInt::get(type, expr->value), type};
  }
  ExpressionInfo Visitor::lvalue(CharLitExpr* expr) { return {}; }

  ExpressionInfo Visitor::rvalue(BoolLitExpr* expr) {
    llvm::Type* type = llvm::Type::getInt1Ty(*context);
    return {llvm::ConstantInt::get(type, expr->value), type};
  }
  ExpressionInfo Visitor::lvalue(BoolLitExpr* expr) { return {}; }

  ExpressionInfo Visitor::rvalue(StrLitExpr* expr) {
    // TODO: double check this
    return builder->CreateGlobalString(expr->value);
  }
  ExpressionInfo Visitor::lvalue(StrLitExpr* expr) { return {}; }

  ExpressionInfo Visitor::rvalue(IdentifierExpr* expr) {
    if (named_variables.find(expr->name) == named_variables.end())
      Report("Use of undeclared identifier \"" + expr->name + "\"\n", true);

    Variable* variable = &named_variables[expr->name];
    llvm::Value* value = variable->value;
    llvm::Type* type = variable->type;

    return {builder->CreateLoad(type, value), type, variable};
  }
  ExpressionInfo Visitor::lvalue(IdentifierExpr* expr) {
    if (named_variables.find(expr->name) == named_variables.end())
      Report("Use of undeclared identifier \"" + expr->name + "\"\n", true);

    Variable* variable = &named_variables[expr->name];
    llvm::Value* value = variable->value;
    llvm::Type* type = variable->type;

    // don't load
    return {value, type, variable};
  }

  ExpressionInfo Visitor::rvalue(BinOpExpr* expr) {
    if (expr->op == TokenType::EQUAL)
      return operation->asgn(expr->left->lvalue(this), expr->right->rvalue(this));

    switch (expr->op) {
      case TokenType::PLUS:
        return operation->add(expr->left->rvalue(this), expr->right->rvalue(this));
      case TokenType::MINUS:
        return operation->sub(expr->left->rvalue(this), expr->right->rvalue(this));
      case TokenType::STAR:
        return operation->mul(expr->left->rvalue(this), expr->right->rvalue(this));
      case TokenType::SLASH:
        return operation->div(expr->left->rvalue(this), expr->right->rvalue(this));
      default:
        Report("invalid binary operator", true);
    }

    return nullptr;
  }
  ExpressionInfo Visitor::lvalue(BinOpExpr* expr) { return {}; }

  ExpressionInfo Visitor::rvalue(RefExpr* expr) {
    Variable* variable = &named_variables[expr->ide->name];
    llvm::Value* ptr = variable->value;
    llvm::Type* type = variable->type;

    if (!ptr)
      Report("Use of undeclared identifier \"" + expr->ide->name + "\"\n", true);

    return {ptr, ptr->getType(), variable};
  }
  ExpressionInfo Visitor::lvalue(RefExpr* expr) { return {}; }

  ExpressionInfo Visitor::rvalue(DeRefExpr* expr) {
    // loaded value
    ExpressionInfo pointer_expr = expr->ptr_expr->rvalue(this);

    if (!pointer_expr.variable)
      Report("Dereferencing operations only supports direct identifiers\n", true);

    if (!pointer_expr.variable->ptr_to_variable)
      Report("Trying to dereference a non-pointer type \"" + pointer_expr.variable->name + "\"\n", true);

    Variable* variable = pointer_expr.variable->ptr_to_variable;

    return {builder->CreateLoad(variable->type, pointer_expr.value), variable->type, variable};
  }
  ExpressionInfo Visitor::lvalue(DeRefExpr* expr) {
    // do not load
    ExpressionInfo pointer_expr = expr->ptr_expr->lvalue(this);

    if (!pointer_expr.variable)
      Report("Dereferencing operations only supports direct identifiers\n", true);

    if (!pointer_expr.variable->ptr_to_variable)
      Report("Trying to dereference a non-pointer type \"" + pointer_expr.variable->name + "\"\n", true);

    Variable* next_variable = pointer_expr.variable->ptr_to_variable;

    return {builder->CreateLoad(pointer_expr.type, pointer_expr.value), pointer_expr.type, next_variable};
  }

  ExpressionInfo Visitor::rvalue(FnCallExpr* expr) {
    llvm::Function* calle_fn = module->getFunction(expr->name);

    if (!calle_fn)
      Report("undefined reference to function call: \"" + expr->name + "\"\n", true);

    if (calle_fn->arg_size() != expr->args.size())
      Report("Incorrect signature for function call: \"" + expr->name + "\"\n", true);

    std::vector<llvm::Value*> args_values;
    args_values.reserve(expr->args.size());

    for (size_t i = 0; i < expr->args.size(); ++i) {
      ExpressionInfo arg_expr = expr->args[i]->rvalue(this);

      if (!arg_expr.value)
        Report("Unidentified argument value for function \"" + expr->name + "\"\n", true);

      args_values.push_back(arg_expr.value);
    }

    return {builder->CreateCall(calle_fn, args_values, expr->name), calle_fn->getReturnType()};
  }
  ExpressionInfo Visitor::lvalue(FnCallExpr* expr) { return {}; }

  ExpressionInfo Visitor::visit(ReturnStt* stt) {
    llvm::BasicBlock* current_block = builder->GetInsertBlock();

    if (!current_block)
      Report("Incorrect return statement place\n", true);

    llvm::Function* current_function = current_block->getParent();

    if (!current_function)
      Report("return outside of a function is not allowed\n", true);

    if (!stt->expr)
      return {builder->CreateRetVoid(), llvm::Type::getVoidTy(*context)};

    // return value
    ExpressionInfo return_expr = stt->expr->rvalue(this);

    if (!return_expr.value)
      Report("Unexpected error in return statement for function \"" + std::string(current_function->getName()) + "\"\n", true);

    // complete here
    llvm::Value* return_value = return_expr.value;

    if (!return_value)
      return nullptr;

    // current function return type
    llvm::Type* fn_return_type = current_function->getReturnType();

    std::string cast_error = "Invalid operation: casting \"" +
                             get_string_type(return_value->getType()) + "\" to \"" +
                             get_string_type(fn_return_type) + "\", for function return \"" +
                             std::string(current_block->getParent()->getName()) + "\"\n";

    if (fn_return_type != return_value->getType())
      return_value = cast(return_value, fn_return_type, cast_error);

    return builder->CreateRet(return_value);
  }

  ExpressionInfo Visitor::visit(ExprStt* stt) {
    return stt->expr->rvalue(this);
  }

  ExpressionInfo Visitor::visit(VarDecStt* stt) {
    if (named_variables.find(stt->variable.name) != named_variables.end())
      Report("Variable redefinition for \"" + stt->variable.name + "\"\n", true);

    // global variable
    if (!builder->GetInsertBlock())
      return global_var_dec(stt);

    // local variable
    return local_var_dec(stt);
  }

  ExpressionInfo Visitor::visit(FnDecStt* stt) {
    // Convert parameter types
    std::vector<llvm::Type*> param_types;
    param_types.reserve(stt->params.size());

    for (auto& param : stt->params) {
      ExpressionInfo param_info = param->initializer->rvalue(this);

      param->variable.type = param_info.type;
      llvm::Type* param_type = param->variable.type;

      if (!param_type)
        Report("Unrecognized data type \"" + get_string_type(param_type) + "\": \"" + param->variable.name + "\"\n", true);

      param_types.push_back(param_type);
    }

    // Get return type
    llvm::Type* return_type = get_llvm_type(stt->type);

    if (!return_type)
      Report("Unrecognized return type \"" + stt->type + "\": \"" + stt->name + "\"\n", true);

    // Create function type
    llvm::FunctionType* fn_type = llvm::FunctionType::get(
        return_type, // return type
        param_types, // prameters types
        false        // variable argument
    );

    // Create function
    llvm::Function* fn = llvm::Function::Create(
        fn_type,
        llvm::Function::ExternalLinkage,
        0,
        stt->name,
        module.get());

    return {fn, fn_type};
  }

  ExpressionInfo Visitor::visit(FnDefStt* stt) {
    llvm::Function* fn = module->getFunction(stt->declaration->name);

    if (!fn)
      fn = llvm::dyn_cast<llvm::Function>(stt->declaration->accept(this).value);

    if (!fn)
      Report("Can't declare function \"" + stt->declaration->type + "\": \"" + stt->declaration->name + "\"\n", true);

    // has no body
    if (!fn->empty())
      Report("Function cannot be redefined \"" + stt->declaration->type + "\": \"" + stt->declaration->name + "\"\n", true);

    // Create basic block for function body
    llvm::BasicBlock* BB = llvm::BasicBlock::Create(*context, "entry", fn);
    builder->SetInsertPoint(BB);

    /*
     * NOTE: for nested scopes, save the named_values as "old_named_values"
     * add the arguments of the functions to the named_values, generate the code
     * for function body, restore the "old_named_values" in named_values.
     */
    auto old_named_values = named_variables;

    // arguments handling
    size_t idx = 0;
    for (auto& arg : fn->args()) {
      llvm::AllocaInst* alloca = builder->CreateAlloca(arg.getType());
      builder->CreateStore(&arg, alloca);

      std::string name = stt->declaration->params[idx]->variable.name;
      named_variables[name] = Variable(name, alloca, alloca->getAllocatedType());
      ++idx;
    }

    // Generate code for function body
    // note: each statement has it's own error handling
    // so probably, if an error occurs, the compiler will
    // shut down and we won't came back here
    for (auto& stmt : stt->body)
      stmt->accept(this);

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

      else if (ret_value->isFloatingPointTy())
        builder->CreateRet(llvm::ConstantFP::get(ret_value, 0.0));

      else if (ret_value->isPointerTy())
        builder->CreateRet(llvm::Constant::getNullValue(ret_value));

      else
        builder->CreateUnreachable();
    }

    // clear this scope's variables
    named_variables = old_named_values;

    // back to the previous scope
    builder->ClearInsertionPoint();

    // Verify the function
    if (llvm::verifyFunction(*fn, &llvm::errs())) {
      Report("Function verification failed for \"" + stt->declaration->type +
             "\": \"" + stt->declaration->name + "\"\n");

      fn->eraseFromParent();
      return nullptr;
    }

    // optimize the function
    if (FPM && FAM)
      FPM->run(*fn, *FAM);

    return {fn};
  }
} // namespace phantom
