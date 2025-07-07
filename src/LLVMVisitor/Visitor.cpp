#include <LLVMVisitor/Visitor.hpp>

namespace phantom {
  Visitor::Visitor(const std::string& module_name) {
    context = std::make_unique<llvm::LLVMContext>();
    module = std::make_unique<llvm::Module>(module_name, *context);

    builder = std::make_unique<llvm::IRBuilder<>>(*context);
  }

  void Visitor::print_representation() const {
    module->print(llvm::outs(), nullptr);
  }

  ExpressionInfo Visitor::create_addition(ExpressionInfo left, ExpressionInfo right) {
    llvm::Value* left_val = left.value;
    llvm::Value* right_val = right.value;

    llvm::Value* result = nullptr;

    if (!left_val || !right_val)
      return nullptr;

    if (left_val->getType()->isFloatingPointTy() && right_val->getType()->isPointerTy() ||
        left_val->getType()->isPointerTy() && right_val->getType()->isFloatingPointTy())
      Report("Invalid pointer arithmetic expression: (ptr +- f) or (f +- ptr) is undefined\n", true);

    else if (left_val->getType()->isIntegerTy() && right_val->getType()->isIntegerTy())
      result = builder->CreateAdd(left_val, right_val);

    else if (left_val->getType()->isFloatingPointTy() || right_val->getType()->isFloatingPointTy()) {
      if (left_val->getType()->isIntegerTy())
        left_val = builder->CreateSIToFP(left_val, builder->getFloatTy());

      if (right_val->getType()->isIntegerTy())
        right_val = builder->CreateSIToFP(right_val, builder->getFloatTy());

      result = builder->CreateFAdd(left_val, right_val);
    }

    else if (left_val->getType()->isPointerTy() && right_val->getType()->isIntegerTy()) {
      // for pointers they return the ptr_to_type in ExpressionInfo.type
      llvm::Type* ptr_to_type = left.type;

      if (!ptr_to_type)
        Report("Internal compiler error: Unexpected pointer-to type\n");

      result = builder->CreateGEP(ptr_to_type, left_val, right_val);
    }

    else if (left_val->getType()->isIntegerTy() && right_val->getType()->isPointerTy()) {
      // for pointers they return the ptr_to_type in ExpressionInfo.type
      llvm::Type* ptr_to_type = right.type;

      if (!ptr_to_type)
        Report("Internal compiler error: Unexpected pointer-to type\n");

      result = builder->CreateGEP(ptr_to_type, right_val, left_val);
    }

    else if (left_val->getType()->isPointerTy() && right_val->getType()->isPointerTy())
      Report("Invalid pointer arithmetic expression: ptr + ptr is undefined\n", true);

    if (!result)
      Report("Unhandled types for left and right of an assignment expression\n", true);

    return {result, result->getType()};
  }

  ExpressionInfo Visitor::create_substraction(ExpressionInfo left, ExpressionInfo right) {
    llvm::Value* left_val = left.value;
    llvm::Value* right_val = right.value;

    llvm::Value* result = nullptr;

    if (!left_val || !right_val)
      return nullptr;

    if (left_val->getType()->isFloatingPointTy() && right_val->getType()->isPointerTy() ||
        left_val->getType()->isPointerTy() && right_val->getType()->isFloatingPointTy())
      Report("Invalid pointer arithmetic expression: (ptr +- f) or (f +- ptr) is undefined\n", true);

    else if (left_val->getType()->isIntegerTy() && right_val->getType()->isIntegerTy())
      result = builder->CreateSub(left_val, right_val);

    if (left_val->getType()->isFloatingPointTy() || right_val->getType()->isFloatingPointTy()) {
      if (left_val->getType()->isIntegerTy())
        left_val = builder->CreateSIToFP(left_val, builder->getFloatTy());

      if (right_val->getType()->isIntegerTy())
        right_val = builder->CreateSIToFP(right_val, builder->getFloatTy());

      result = builder->CreateFSub(left_val, right_val);
    }

    // distance between pointers
    else if (left_val->getType()->isPointerTy() && right_val->getType()->isPointerTy()) {
      // for pointers they return the ptr_to_type in ExpressionInfo.type
      llvm::Type* left_type = left.type;
      llvm::Type* right_type = right.type;

      if (!left_type || !right_type)
        Report("Unknown pointer-to type\n", true);

      if (left_type != right_type)
        Report("Invalid pointer arithmetic expression: ptr - ptr for different pointed data types is undefined\n", true);

      llvm::Value* left_int = builder->CreatePtrToInt(left_val, builder->getInt64Ty());
      llvm::Value* right_int = builder->CreatePtrToInt(right_val, builder->getInt64Ty());
      llvm::Value* byte_diff = builder->CreateSub(left_int, right_int);

      uint64_t element_size = module->getDataLayout().getTypeAllocSize(left_type);
      llvm::Value* size_value = llvm::ConstantInt::get(builder->getInt64Ty(), element_size);

      result = builder->CreateSDiv(byte_diff, size_value);
    }

    else if (left_val->getType()->isPointerTy() && right_val->getType()->isIntegerTy()) {
      llvm::Type* type = left.type;

      if (!type)
        Report("Undefined pointer-to type\n", true);

      result = builder->CreateGEP(type, left_val, builder->CreateNeg(right_val));
    }

    else if (left_val->getType()->isIntegerTy() && right_val->getType()->isPointerTy())
      Report("Invalid pointer arithmetic expression: n - ptr is undefined\n", true);

    if (!result)
      Report("Unhandled types for left and right of an assignment expression\n", true);

    return {result, result->getType()};
  }

  ExpressionInfo Visitor::create_multiplication(ExpressionInfo left, ExpressionInfo right) {
    llvm::Value* left_val = left.value;
    llvm::Value* right_val = right.value;

    llvm::Value* result = nullptr;

    if (!left_val || !right_val)
      return nullptr;

    if (left_val->getType()->isPointerTy() || right_val->getType()->isPointerTy())
      Report("Invalid pointer arithmetic expression: ptr * n/f/ptr is undefined\n", true);

    else if (left_val->getType()->isIntegerTy() && right_val->getType()->isIntegerTy())
      result = builder->CreateMul(left_val, right_val);

    else if (left_val->getType()->isFloatingPointTy() || right_val->getType()->isFloatingPointTy())
      result = builder->CreateFMul(left_val, right_val);

    return {result, result->getType()};
  }

  ExpressionInfo Visitor::create_division(ExpressionInfo left, ExpressionInfo right) {
    llvm::Value* left_val = left.value;
    llvm::Value* right_val = right.value;

    llvm::Value* result = nullptr;

    if (!left_val || !right_val)
      return nullptr;

    if (left_val->getType()->isPointerTy() || right_val->getType()->isPointerTy())
      Report("Invalid pointer arithmetic expression: ptr / n/f/ptr is undefined\n", true);

    if (left_val->getType()->isIntegerTy() && right_val->getType()->isIntegerTy())
      result = builder->CreateSDiv(left_val, right_val);

    if (left_val->getType()->isFloatingPointTy() || right_val->getType()->isFloatingPointTy())
      result = builder->CreateFDiv(left_val, right_val);

    return {result, result->getType()};
  }

  ExpressionInfo Visitor::assign(IdentifierExpr* left, ExpressionInfo right) {
    if (named_variables.find(left->name) == named_variables.end())
      Report("Use of undeclared identifier \"" + left->name + "\"\n", true);

    Variable* variable = &named_variables[left->name];
    builder->CreateStore(right.value, variable->value);

    return right;
  }

  ExpressionInfo Visitor::assign(DeRefExpr* left, ExpressionInfo right) {
    IdentifierExpr* ide = dynamic_cast<IdentifierExpr*>(left->ptr_expr.get());

    if (!ide)
      Report("Dereferencing operations only supports direct identifiers\n", true);

    Variable* variable = &named_variables[ide->name];

    if (!variable || !variable->value || !variable->type)
      Report("Undefined dereference to variable \"" + ide->name + "\"\n", true);

    if (!variable->type->isPointerTy())
      Report("Trying to dereference a non-pointer type \"" + ide->name + "\"\n", true);

    llvm::Value* ptr_value = builder->CreateLoad(variable->type, variable->value);

    builder->CreateStore(right.value, ptr_value);

    return right;
  }

  ExpressionInfo Visitor::handle_assignment(Expression* left, ExpressionInfo right) {
    llvm::Value* ptr = nullptr;

    if (auto* ide = dynamic_cast<IdentifierExpr*>(left))
      return ide->assign(right, this);

    else if (auto* deref = dynamic_cast<DeRefExpr*>(left))
      return deref->assign(right, this);

    Report("Unexpected assignment operation: left side is not assignable\n", true);

    return nullptr;
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
    std::string raw_type = stt->variable.raw_type;

    if (raw_type.empty() && !stt->initializer)
      Report("Can not initialize type for variable \"" + name + "\"\n", true);

    if (!raw_type.empty())
      variable_type = get_llvm_type(raw_type);

    if (stt->initializer) {
      ExpressionInfo init = stt->initializer->accept(this);

      if (!init.value || !init.type)
        Report("Internal compiler error: not enough informations about variable\"" + name + "\"\n", true);

      value = init.value;

      if (!variable_type)
        variable_type = value->getType();

      if (variable_type->isPointerTy())
        stt->variable.ptr_to_type = init.type;

      else if (variable_type != value->getType()) {
        std::string cast_error = "Invalid operation: casting \"" +
                                 get_string_type(value->getType()) + "\" to \"" +
                                 get_string_type(variable_type) + "\", for variable \"" + name + "\"\n";

        value = cast(value, variable_type, cast_error);
      }

      constant_init = llvm::dyn_cast<llvm::Constant>(value);
    }

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
    return {global_variable, variable_type};
  }

  ExpressionInfo Visitor::local_var_dec(VarDecStt* stt) {
    llvm::Type* variable_type = nullptr;
    llvm::Value* value = nullptr;

    std::string name = stt->variable.name;
    std::string raw_type = stt->variable.raw_type;

    if (raw_type.empty() && !stt->initializer)
      Report("Can not initialize type for variable \"" + name + "\"\n", true);

    if (!raw_type.empty())
      variable_type = get_llvm_type(raw_type);

    if (stt->initializer) {
      ExpressionInfo init = stt->initializer->accept(this);

      if (!init.value || !init.type)
        Report("Internal compiler error: not enough informations about variable\"" + name + "\"\n", true);

      value = init.value;

      if (!variable_type)
        variable_type = value->getType();

      if (variable_type->isPointerTy())
        stt->variable.ptr_to_type = init.type;

      else if (variable_type != value->getType()) {
        std::string cast_error = "Invalid operation: casting \"" +
                                 get_string_type(value->getType()) + "\" to \"" +
                                 get_string_type(variable_type) + "\", for variable \"" + name + "\"\n";

        value = cast(value, variable_type, cast_error);
      }
    }

    // create the alloca instruction
    llvm::AllocaInst* alloca = builder->CreateAlloca(variable_type, nullptr);

    stt->variable.value = alloca;
    stt->variable.type = variable_type;

    if (value)
      builder->CreateStore(value, alloca);

    // Store in symbol table for later lookups
    named_variables[name] = stt->variable;

    // Return the alloca instruction (which is a Value*)
    return {alloca, variable_type};
  }

  // ----------------------- visit functions -------------------------- //

  ExpressionInfo Visitor::visit(IntLitExpr* expr) {
    llvm::Type* type = nullptr;

    if (expr->value >= INT_MIN_VAL && expr->value <= INT_MAX_VAL)
      type = llvm::Type::getInt32Ty(*context);

    else if (expr->value >= LONG_MIN_VAL && expr->value <= LONG_MAX_VAL)
      type = llvm::Type::getInt64Ty(*context);

    else
      Report("Value is too large to be represented in a data type\n", true);

    return {llvm::ConstantInt::get(type, expr->value), type};
  }

  ExpressionInfo Visitor::visit(FloatLitExpr* expr) {
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

    return {llvm::ConstantFP::get(type, expr->value), type};
  }

  ExpressionInfo Visitor::visit(CharLitExpr* expr) {
    llvm::Type* type = llvm::Type::getInt8Ty(*context);
    return {llvm::ConstantInt::get(type, expr->value), type};
  }

  ExpressionInfo Visitor::visit(BoolLitExpr* expr) {
    llvm::Type* type = llvm::Type::getInt1Ty(*context);
    return {llvm::ConstantInt::get(type, expr->value), type};
  }

  ExpressionInfo Visitor::visit(StrLitExpr* expr) {
    // TODO: double check this
    return builder->CreateGlobalString(expr->value);
  }

  ExpressionInfo Visitor::visit(IdentifierExpr* expr) {
    if (named_variables.find(expr->name) == named_variables.end())
      Report("Use of undeclared identifier \"" + expr->name + "\"\n", true);

    Variable* variable = &named_variables[expr->name];
    llvm::Value* value = variable->value;
    llvm::Type* type = variable->type;

    return {builder->CreateLoad(type, value), type, variable};
  }

  ExpressionInfo Visitor::visit(BinOpExpr* expr) {
    if (expr->op == TokenType::EQUAL)
      return handle_assignment(expr->left.get(), expr->right->accept(this));

    switch (expr->op) {
      case TokenType::PLUS:
        return create_addition(expr->left->accept(this), expr->right->accept(this));
      case TokenType::MINUS:
        return create_substraction(expr->left->accept(this), expr->right->accept(this));
      case TokenType::STAR:
        return create_multiplication(expr->left->accept(this), expr->right->accept(this));
      case TokenType::SLASH:
        return create_division(expr->left->accept(this), expr->right->accept(this));
      default:
        Report("invalid binary operator", true);
    }

    return nullptr;
  }

  ExpressionInfo Visitor::visit(RefExpr* expr) {
    Variable* variable = &named_variables[expr->ide->name];
    llvm::Value* ptr = variable->value;
    llvm::Type* type = variable->type;

    if (!ptr)
      Report("Use of undeclared identifier \"" + expr->ide->name + "\"\n", true);

    return {ptr, type, variable};
  }

  ExpressionInfo Visitor::visit(DeRefExpr* expr) {
    IdentifierExpr* ide = dynamic_cast<IdentifierExpr*>(expr->ptr_expr.get());

    if (!ide)
      Report("Dereferencing operations only supports direct identifiers\n", true);

    Variable* variable = &named_variables[ide->name];

    if (!variable || !variable->value || !variable->type)
      Report("Undefined dereference to variable \"" + ide->name + "\"\n", true);

    if (!variable->type->isPointerTy())
      Report("Trying to dereference a non-pointer type \"" + ide->name + "\"\n", true);

    llvm::Value* ptr_value = builder->CreateLoad(variable->type, variable->value);

    return {builder->CreateLoad(variable->ptr_to_type, ptr_value), variable->ptr_to_type};
  }

  ExpressionInfo Visitor::visit(FnCallExpr* expr) {
    llvm::Function* calle_fn = module->getFunction(expr->name);

    if (!calle_fn)
      Report("undefined reference to function call: \"" + expr->name + "\"\n", true);

    if (calle_fn->arg_size() != expr->args.size())
      Report("Incorrect signature for function call: \"" + expr->name + "\"\n", true);

    std::vector<llvm::Value*> args_values;
    args_values.reserve(expr->args.size());

    for (size_t i = 0; i < expr->args.size(); ++i) {
      ExpressionInfo arg_expr = expr->args[i]->accept(this);

      if (!arg_expr.value)
        Report("Unidentified argument value for function \"" + expr->name + "\"\n", true);

      args_values.push_back(arg_expr.value);
    }

    return {builder->CreateCall(calle_fn, args_values, expr->name), calle_fn->getReturnType()};
  }

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
    ExpressionInfo return_expr = stt->expr->accept(this);

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
    return stt->expr->accept(this);
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
      param.type = get_llvm_type(param.raw_type);
      llvm::Type* param_type = param.type;

      if (!param_type)
        Report("Unrecognized data type \"" + param.raw_type + "\": \"" + param.name + "\"\n", true);

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

      std::string name = stt->declaration->params[idx].name;
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

    return {fn};
  }
} // namespace phantom
