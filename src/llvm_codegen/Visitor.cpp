#include <ast/Statement.hpp>
#include <llvm_codegen/Visitor.hpp>

#include "llvm/IR/Verifier.h"

namespace phantom {
  namespace llvm_codegen {
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

    llvm::Type* Visitor::get_llvm_type(std::string type, bool pointer) const {
      if (type == "ptr")
        return llvm::PointerType::get(*context, 0);
      else if (type == "void")
        return llvm::Type::getVoidTy(*context);
      else if (type == "bool")
        return llvm::Type::getInt1Ty(*context);
      else if (type == "char")
        return llvm::Type::getInt8Ty(*context);
      else if (type == "short")
        return llvm::Type::getInt16Ty(*context);
      else if (type == "int")
        return llvm::Type::getInt32Ty(*context);
      else if (type == "long")
        return llvm::Type::getInt64Ty(*context);
      else if (type == "float")
        return llvm::Type::getFloatTy(*context);
      else if (type == "double")
        return llvm::Type::getDoubleTy(*context);
      else if (type == "quad")
        return llvm::Type::getFP128Ty(*context);
      else
        logger.log(Logger::Level::FATAL, "Unidentified data type: '" + type + "'", true);

      return nullptr;
    }

    std::string Visitor::get_string_type(llvm::Type* type) const {
      if (type->isPointerTy())
        return "ptr";
      else if (type->isIntegerTy(1))
        return "bool";
      else if (type->isIntegerTy(8))
        return "char";
      else if (type->isIntegerTy(16))
        return "short";
      else if (type->isIntegerTy(32))
        return "int";
      else if (type->isIntegerTy(64))
        return "long";
      else if (type->isFP128Ty())
        return "quad";
      else if (type->isDoubleTy())
        return "double";
      else if (type->isFloatTy())
        return "float";
      else if (type->isVoidTy())
        return "void";
      else
        logger.log(Logger::Level::FATAL, "Unexpected llvm type", true);

      return "";
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
        logger.log(Logger::Level::ERROR, error_msg, true);

      return nullptr;
    }

    ExprInfo Visitor::global_var_dec(VarDecStt* stt) {
      llvm::Constant* constant_init = nullptr;
      llvm::Type* variable_type = nullptr;
      llvm::Value* value = nullptr;

      std::string name = stt->variable.name;

      if (!stt->initializer)
        logger.log(Logger::Level::ERROR, "Can not initialize type for variable '" + name + "'", true);

      // initializer available
      ExprInfo init = stt->initializer->rvalue(this);

      if (!init.value || !init.type)
        logger.log(Logger::Level::FATAL, "Internal compiler error: not enough informations about variable '" + name + "'", true);

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

    ExprInfo Visitor::local_var_dec(VarDecStt* stt) {
      llvm::Type* variable_type = nullptr;
      llvm::Value* value = nullptr;

      std::string name = stt->variable.name;

      if (!stt->initializer)
        logger.log(Logger::Level::ERROR, "Can not initialize type for variable '" + name + "'", true);

      // initializer available
      ExprInfo init = stt->initializer->rvalue(this);

      if (!init.value || !init.type)
        logger.log(Logger::Level::FATAL, "Internal compiler error: not enough informations about variable '" + name + "'", true);

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

    ExprInfo Visitor::rvalue(DataTypeExpr* expr) {
      llvm::Value* value = nullptr;
      llvm::Type* type = get_llvm_type(expr->type);

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
        logger.log(Logger::Level::ERROR, "Unknown variable type: '" + expr->type + "'", true);

      return {value, type};
    }
    ExprInfo Visitor::lvalue(DataTypeExpr* expr) { return {}; }

    ExprInfo Visitor::rvalue(ArrTypeExpr* expr) {
      return {};
    }
    ExprInfo Visitor::lvalue(ArrTypeExpr* expr) { return {}; }

    ExprInfo Visitor::rvalue(IntLitExpr* expr) {
      llvm::Type* type = nullptr;

      if (expr->value >= INT_MIN_VAL && expr->value <= INT_MAX_VAL)
        type = llvm::Type::getInt32Ty(*context);

      else if (expr->value >= LONG_MIN_VAL && expr->value <= LONG_MAX_VAL)
        type = llvm::Type::getInt64Ty(*context);

      else
        logger.log(Logger::Level::ERROR, "Value is too large to be represented in a data type", true);

      return {llvm::ConstantInt::get(type, expr->value), type};
    }
    ExprInfo Visitor::lvalue(IntLitExpr* expr) { return {}; }

    ExprInfo Visitor::rvalue(FloatLitExpr* expr) {
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
        logger.log(Logger::Level::ERROR, "Value is too large to be represented in a data type", true);

      return {llvm::ConstantFP::get(type, expr->value), type};
    }
    ExprInfo Visitor::lvalue(FloatLitExpr* expr) { return {}; }

    ExprInfo Visitor::rvalue(CharLitExpr* expr) {
      llvm::Type* type = llvm::Type::getInt8Ty(*context);
      return {llvm::ConstantInt::get(type, expr->value), type};
    }
    ExprInfo Visitor::lvalue(CharLitExpr* expr) { return {}; }

    ExprInfo Visitor::rvalue(BoolLitExpr* expr) {
      llvm::Type* type = llvm::Type::getInt1Ty(*context);
      return {llvm::ConstantInt::get(type, expr->value), type};
    }
    ExprInfo Visitor::lvalue(BoolLitExpr* expr) { return {}; }

    ExprInfo Visitor::rvalue(StrLitExpr* expr) {
      // TODO: double check this
      return builder->CreateGlobalString(expr->value);
    }
    ExprInfo Visitor::lvalue(StrLitExpr* expr) { return {}; }

    ExprInfo Visitor::rvalue(ArrLitExpr* expr) {
      if (expr->elements.size() == 0) {
        logger.log(Logger::Level::ERROR, "Empty array literals are not allowed");
        return {};
      }

      bool can_be_const = true;
      std::vector<llvm::Value*> values;
      std::vector<llvm::Constant*> constants;

      values.reserve(expr->elements.size());
      constants.reserve(expr->elements.size());

      llvm::Type* ref_type = expr->elements[0]->rvalue(this).type;

      if (!ref_type)
        logger.log(Logger::Level::FATAL, "internal compiler error: can't evaluate array literal reference type", true);

      for (auto& element : expr->elements) {
        ExprInfo expr = element->rvalue(this);

        if (expr.type != ref_type) {
          logger.log(Logger::Level::ERROR, "Array Literals can't have different data types");
          return {};
        }

        if (!expr.value)
          logger.log(Logger::Level::ERROR, "internal compiler error: can't evaluate array literal member value type", true);

        values.push_back(expr.value);

        if (can_be_const) {
          if (auto constant = llvm::dyn_cast<llvm::Constant>(expr.value))
            constants.push_back(constant);
          else
            can_be_const = false;
        }
      }

      llvm::ArrayType* arr_type = llvm::ArrayType::get(ref_type, expr->elements.size());
      if (can_be_const)
        return {llvm::ConstantArray::get(arr_type, constants), arr_type};

      // non-const arrays
      if (!builder->GetInsertBlock()) {
        logger.log(Logger::Level::ERROR, "non-const global arrays are not allowed");
        return {};
      }

      llvm::AllocaInst* arr = builder->CreateAlloca(arr_type);

      for (size_t i = 0; i < values.size(); ++i) {
        llvm::Value* element_ptr = builder->CreateConstInBoundsGEP2_32(arr_type, arr, 0, i);
        builder->CreateStore(values[i], element_ptr);
      }

      return {arr, arr_type};
    }
    ExprInfo Visitor::lvalue(ArrLitExpr* expr) { return {}; }

    ExprInfo Visitor::rvalue(IdeExpr* expr) {
      if (named_variables.find(expr->name) == named_variables.end())
        logger.log(Logger::Level::ERROR, "Use of undeclared identifier '" + expr->name + "'", true);

      Variable* variable = &named_variables[expr->name];
      llvm::Value* value = variable->value;
      llvm::Type* type = variable->type;

      return {builder->CreateLoad(type, value), type, variable};
    }
    ExprInfo Visitor::lvalue(IdeExpr* expr) {
      if (named_variables.find(expr->name) == named_variables.end())
        logger.log(Logger::Level::ERROR, "Use of undeclared identifier '" + expr->name + "'", true);

      Variable* variable = &named_variables[expr->name];
      llvm::Value* value = variable->value;
      llvm::Type* type = variable->type;

      // don't load
      return {value, type, variable};
    }

    ExprInfo Visitor::rvalue(BinOpExpr* expr) {
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
          logger.log(Logger::Level::ERROR, "invalid binary operator", true);
      }

      return nullptr;
    }
    ExprInfo Visitor::lvalue(BinOpExpr* expr) { return {}; }

    ExprInfo Visitor::rvalue(RefExpr* expr) {
      Variable* variable = &named_variables[expr->ide->name];
      llvm::Value* ptr = variable->value;
      llvm::Type* type = variable->type;

      if (!ptr)
        logger.log(Logger::Level::ERROR, "Use of undeclared identifier '" + expr->ide->name + "'", true);

      return {ptr, ptr->getType(), variable};
    }
    ExprInfo Visitor::lvalue(RefExpr* expr) { return {}; }

    ExprInfo Visitor::rvalue(DeRefExpr* expr) {
      // loaded value
      ExprInfo pointer_expr = expr->ptr_expr->rvalue(this);

      if (!pointer_expr.variable)
        logger.log(Logger::Level::ERROR, "Dereferencing operations only supports direct identifiers", true);

      if (!pointer_expr.variable->ptr_to_variable)
        logger.log(Logger::Level::ERROR, "Trying to dereference a non-pointer type '" + pointer_expr.variable->name + "'", true);

      Variable* variable = pointer_expr.variable->ptr_to_variable;

      return {builder->CreateLoad(variable->type, pointer_expr.value), variable->type, variable};
    }
    ExprInfo Visitor::lvalue(DeRefExpr* expr) {
      // do not load
      ExprInfo pointer_expr = expr->ptr_expr->lvalue(this);

      if (!pointer_expr.variable)
        logger.log(Logger::Level::ERROR, "Dereferencing operations only supports direct identifiers", true);

      if (!pointer_expr.variable->ptr_to_variable)
        logger.log(Logger::Level::ERROR, "Trying to dereference a non-pointer type '" + pointer_expr.variable->name + "'", true);

      Variable* next_variable = pointer_expr.variable->ptr_to_variable;

      return {builder->CreateLoad(pointer_expr.type, pointer_expr.value), pointer_expr.type, next_variable};
    }

    ExprInfo Visitor::rvalue(FnCallExpr* expr) {
      llvm::Function* calle_fn = module->getFunction(expr->name);

      if (!calle_fn)
        logger.log(Logger::Level::ERROR, "undefined reference to function call: '" + expr->name + "'", true);

      if (calle_fn->arg_size() != expr->args.size())
        logger.log(Logger::Level::ERROR, "Incorrect signature for function call: '" + expr->name + "'", true);

      std::vector<llvm::Value*> args_values;
      args_values.reserve(expr->args.size());

      for (size_t i = 0; i < expr->args.size(); ++i) {
        ExprInfo arg_expr = expr->args[i]->rvalue(this);

        if (!arg_expr.value)
          logger.log(Logger::Level::ERROR, "Unidentified argument value for function '" + expr->name + "'", true);

        args_values.push_back(arg_expr.value);
      }

      return {builder->CreateCall(calle_fn, args_values, expr->name), calle_fn->getReturnType()};
    }
    ExprInfo Visitor::lvalue(FnCallExpr* expr) { return {}; }

    ExprInfo Visitor::visit(ReturnStt* stt) {
      llvm::BasicBlock* current_block = builder->GetInsertBlock();

      if (!current_block)
        logger.log(Logger::Level::ERROR, "Incorrect return statement place", true);

      llvm::Function* current_function = current_block->getParent();

      if (!current_function)
        logger.log(Logger::Level::ERROR, "return outside of a function is not allowed", true);

      if (!stt->expr)
        return {builder->CreateRetVoid(), llvm::Type::getVoidTy(*context)};

      // return value
      ExprInfo return_expr = stt->expr->rvalue(this);

      if (!return_expr.value)
        logger.log(Logger::Level::ERROR, "Unexpected error in return statement for function '" + std::string(current_function->getName()) + "'", true);

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

    ExprInfo Visitor::visit(ExprStt* stt) {
      return stt->expr->rvalue(this);
    }

    ExprInfo Visitor::visit(VarDecStt* stt) {
      if (named_variables.find(stt->variable.name) != named_variables.end())
        logger.log(Logger::Level::ERROR, "Variable redefinition for '" + stt->variable.name + "'", true);

      // global variable
      if (!builder->GetInsertBlock())
        return global_var_dec(stt);

      // local variable
      return local_var_dec(stt);
    }

    ExprInfo Visitor::visit(FnDecStt* stt) {
      // Convert parameter types
      std::vector<llvm::Type*> param_types;
      param_types.reserve(stt->params.size());

      for (auto& param : stt->params) {
        ExprInfo param_info = param->initializer->rvalue(this);

        param->variable.type = param_info.type;
        llvm::Type* param_type = param->variable.type;

        if (!param_type)
          logger.log(Logger::Level::ERROR, "Unrecognized data type '" + get_string_type(param_type) + "': '" + param->variable.name + "'", true);

        param_types.push_back(param_type);
      }

      // Get return type
      llvm::Type* return_type = get_llvm_type(stt->type);

      if (!return_type)
        logger.log(Logger::Level::ERROR, "Unrecognized return type '" + stt->type + "': '" + stt->name + "'", true);

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

    ExprInfo Visitor::visit(FnDefStt* stt) {
      llvm::Function* fn = module->getFunction(stt->declaration->name);

      if (!fn)
        fn = llvm::dyn_cast<llvm::Function>(stt->declaration->accept(this).value);

      if (!fn)
        logger.log(Logger::Level::ERROR, "Can't declare function '" + stt->declaration->type + "' : '" + stt->declaration->name + "'", true);

      // has no body
      if (!fn->empty())
        logger.log(Logger::Level::ERROR, "Function cannot be redefined '" + stt->declaration->type + "' : '" + stt->declaration->name + "'", true);

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
        named_variables[name] = Variable(name, false, alloca, alloca->getAllocatedType());
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
        logger.log(Logger::Level::ERROR, "Function verification failed for '" + stt->declaration->type + "' : '" + stt->declaration->name + "'", true);

        fn->eraseFromParent();
        return nullptr;
      }

      // optimize the function
      if (FPM && FAM)
        FPM->run(*fn, *FAM);

      return {fn};
    }
  } // namespace llvm_codegen
} // namespace phantom
