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

    Data Visitor::create_local_variable(VarDecExpr* expr) {
      std::string name = expr->variable.name;

      if (named_variables.find(name) != named_variables.end())
        logger.log(Logger::Level::WARNING, "Redefinition of an already existing variable '" + name + "'");

      llvm::Type* type = nullptr;
      llvm::Value* value = nullptr;

      if (!expr->initializer) {
        logger.log(Logger::Level::ERROR, "Can not initialize type for variable '" + name + "'");
        return {};
      }

      // initializer available
      Data init = expr->initializer->rvalue(this);

      if (!init.value || !init.type) {
        logger.log(Logger::Level::FATAL, "Internal compiler error: not enough informations about variable '" + name + "'", true);
        return {};
      }

      value = init.value;
      type = init.type;

      // pointers case
      if (type->isPointerTy() && init.variable) {
        if (!init.variable) {
          logger.log(Logger::Level::ERROR, "internal compiler error: missing variable for a pointer type");
          return {};
        }
        expr->variable.ptr_to_variable = init.variable;
      }

      // create the alloca instruction
      llvm::AllocaInst* alloca = builder->CreateAlloca(type, nullptr);
      builder->CreateStore(value, alloca);

      expr->variable.value = alloca;
      expr->variable.type = type;

      // Store in symbol table for later lookups
      named_variables[name] = expr->variable;

      // Return the alloca instruction (which is a Value*)
      return {alloca, type, &named_variables[name]};
    }

    Data Visitor::create_global_variable(VarDecExpr* expr) {
      std::string name = expr->variable.name;

      if (named_variables.find(name) != named_variables.end())
        logger.log(Logger::Level::WARNING, "Redefinition of an already existing variable '" + name + "'");

      llvm::Type* type = nullptr;
      llvm::Constant* value = nullptr;

      if (!expr->initializer) {
        logger.log(Logger::Level::ERROR, "Can not initialize type for variable '" + name + "'");
        return {};
      }

      // initializer available
      Data init = expr->initializer->rvalue(this);

      if (!init.value || !init.type) {
        logger.log(Logger::Level::FATAL, "Internal compiler error: not enough informations about variable '" + name + "'", true);
        return {};
      }

      if (!(value = llvm::dyn_cast<llvm::Constant>(init.value))) {
        logger.log(Logger::Level::ERROR, "global variables should have a constant initializer, '" + name + "'");
        return {};
      }

      type = init.type;

      // pointers case
      if (type->isPointerTy() && init.variable) {
        if (!init.variable) {
          logger.log(Logger::Level::ERROR, "internal compiler error: missing Variable for a pointer type");
          return {};
        }
        expr->variable.ptr_to_variable = init.variable;
      }

      // declare the global variable
      llvm::GlobalVariable* global_variable = new llvm::GlobalVariable(
          *module,                            // module
          type,                               // type
          false,                              // constant
          llvm::GlobalValue::ExternalLinkage, // linkage
          value,                              // Constant
          name                                // name
      );

      expr->variable.value = global_variable;
      expr->variable.type = type;
      expr->variable.global = true;

      named_variables[name] = expr->variable;
      return {global_variable, type, &named_variables[name]};
    }

    Data Visitor::create_local_array(ArrDecExpr* expr) {
      std::string name = expr->variable.name;

      if (named_variables.find(name) != named_variables.end())
        logger.log(Logger::Level::WARNING, "Redefinition of an already existing variable '" + name + "'");

      if (!expr->initializer) {
        logger.log(Logger::Level::ERROR, "Can not initialize type for variable '" + name + "'");
        return {};
      }

      // initializer available
      Data init = expr->initializer->rvalue(this);

      if (!init.value || !init.type) {
        logger.log(Logger::Level::FATAL, "Internal compiler error: not enough informations about variable '" + name + "'", true);
        return {};
      }

      expr->variable.value = init.value;
      expr->variable.type = init.value->getType();

      // Store in symbol table for later lookups
      named_variables[name] = expr->variable;

      // Return the alloca instruction (which is a Value*)
      return {init.value, init.type, &named_variables[name]};
    }

    Data Visitor::create_global_array(ArrDecExpr* expr) {
      std::string name = expr->variable.name;

      if (named_variables.find(name) != named_variables.end())
        logger.log(Logger::Level::WARNING, "Redefinition of an already existing variable '" + name + "'");

      llvm::Type* type = nullptr;
      llvm::Constant* value = nullptr;

      if (!expr->initializer) {
        logger.log(Logger::Level::ERROR, "Can not initialize type for variable '" + name + "'");
        return {};
      }

      // initializer available
      Data init = expr->initializer->rvalue(this);

      if (!init.value || !init.type) {
        logger.log(Logger::Level::FATAL, "Internal compiler error: not enough informations about variable '" + name + "'", true);
        return {};
      }

      if (!(value = llvm::dyn_cast<llvm::Constant>(init.value))) {
        logger.log(Logger::Level::ERROR, "global variables should have a constant initializer, '" + name + "'");
        return {};
      }

      type = init.type;

      // pointers case
      if (type->isPointerTy() && init.variable) {
        if (!init.variable) {
          logger.log(Logger::Level::ERROR, "internal compiler error: missing Variable for a pointer type");
          return {};
        }
        expr->variable.ptr_to_variable = init.variable;
      }

      // declare the global variable
      llvm::GlobalVariable* global_variable = new llvm::GlobalVariable(
          *module,                            // module
          type,                               // type
          false,                              // constant
          llvm::GlobalValue::ExternalLinkage, // linkage
          value,                              // Constant
          name                                // name
      );

      expr->variable.value = global_variable;
      expr->variable.type = type;
      expr->variable.global = true;

      named_variables[name] = expr->variable;
      return {global_variable, type, &named_variables[name]};
    }

    Data Visitor::rvalue(DataTypeExpr* expr) {
      llvm::Value* value = nullptr;
      llvm::Type* type = Help::string_to_llvm(expr->type, context, logger);

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
    Data Visitor::lvalue(DataTypeExpr* expr) { return {}; }

    Data Visitor::rvalue(ArrTypeExpr* expr) {
      if (!expr->length && !expr->value) {
        logger.log(Logger::Level::ERROR, "could not determine array length");
        return {};
      }

      if (!expr->length && !expr->type.empty()) {
        logger.log(Logger::Level::ERROR, "type without a length for an array expression is not allowed");
        return {};
      }

      if (expr->type.empty() && expr->length) {
        logger.log(Logger::Level::ERROR, "length without a type for an array expression is not allowed");
        return {};
      }

      // let x = [1, 2, 3];
      if (!expr->length && expr->value)
        return expr->value->rvalue(this);

      // otherwise the length is present
      llvm::Type* element_type = Help::string_to_llvm(expr->type, context, logger);
      llvm::ArrayType* array_type = nullptr;
      llvm::AllocaInst* array = nullptr;
      size_t array_size = 0;

      if (auto length = llvm::dyn_cast<llvm::ConstantInt>(expr->length->rvalue(this).value)) {
        array_size = length->getSExtValue();
      } else {
        logger.log(Logger::Level::ERROR, "Arrays should have a constant len");
        return {};
      }

      if (array_size == 0) {
        logger.log(Logger::Level::ERROR, "Empty arrays are not allowed");
        return {};
      }

      array_type = llvm::ArrayType::get(element_type, array_size);
      array = builder->CreateAlloca(array_type);

      if (expr->value) {
        ArrLitExpr* array_literal = dynamic_cast<ArrLitExpr*>(expr->value.get());

        if (!array_literal) {
          logger.log(Logger::Level::ERROR, "Only array literals are allowed when the array len is present");
          return {};
        }

        if (array_size < array_literal->elements.size()) {
          logger.log(Logger::Level::ERROR, "Incompatible array len and value elements len");
          return {};
        }

        for (size_t i = 0; i < array_literal->elements.size(); i++) {
          llvm::Value* index = llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context), i);

          llvm::Value* element_ptr = builder->CreateGEP(
              element_type,
              array,
              {index});

          llvm::Value* value = array_literal->elements[i]->rvalue(this).value;
          builder->CreateStore(value, element_ptr);
        }
      }

      return {array, array_type};
    }
    Data Visitor::lvalue(ArrTypeExpr* expr) { return {}; }

    Data Visitor::rvalue(IntLitExpr* expr) {
      llvm::Type* type = nullptr;

      if (expr->value >= INT_MIN_VAL && expr->value <= INT_MAX_VAL)
        type = llvm::Type::getInt32Ty(*context);

      else if (expr->value >= LONG_MIN_VAL && expr->value <= LONG_MAX_VAL)
        type = llvm::Type::getInt64Ty(*context);

      else
        logger.log(Logger::Level::ERROR, "Value is too large to be represented in a data type", true);

      return {llvm::ConstantInt::get(type, expr->value), type};
    }
    Data Visitor::lvalue(IntLitExpr* expr) { return {}; }

    Data Visitor::rvalue(FloatLitExpr* expr) {
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
    Data Visitor::lvalue(FloatLitExpr* expr) { return {}; }

    Data Visitor::rvalue(CharLitExpr* expr) {
      llvm::Type* type = llvm::Type::getInt8Ty(*context);
      return {llvm::ConstantInt::get(type, expr->value), type};
    }
    Data Visitor::lvalue(CharLitExpr* expr) { return {}; }

    Data Visitor::rvalue(BoolLitExpr* expr) {
      llvm::Type* type = llvm::Type::getInt1Ty(*context);
      return {llvm::ConstantInt::get(type, expr->value), type};
    }
    Data Visitor::lvalue(BoolLitExpr* expr) { return {}; }

    Data Visitor::rvalue(StrLitExpr* expr) {
      // TODO: double check this
      return builder->CreateGlobalString(expr->value);
    }
    Data Visitor::lvalue(StrLitExpr* expr) { return {}; }

    Data Visitor::rvalue(ArrLitExpr* expr) {
      if (expr->elements.size() == 0) {
        logger.log(Logger::Level::ERROR, "Empty array literals are not allowed");
        return {};
      }

      std::vector<llvm::Value*> values;
      values.reserve(expr->elements.size());

      llvm::Type* ref_type = expr->elements[0]->rvalue(this).type;

      if (!ref_type)
        logger.log(Logger::Level::FATAL, "internal compiler error: can't evaluate array literal reference type", true);

      for (auto& element : expr->elements) {
        Data expr = element->rvalue(this);

        if (expr.type != ref_type) {
          logger.log(Logger::Level::ERROR, "Array Literals can't have different data types");
          return {};
        }

        if (!expr.value)
          logger.log(Logger::Level::FATAL, "internal compiler error: can't evaluate array literal member value type", true);

        values.push_back(expr.value);
      }

      llvm::ArrayType* arr_type = llvm::ArrayType::get(ref_type, expr->elements.size());

      if (!this->inside_function) {
        std::vector<llvm::Constant*> constants;
        constants.reserve(values.size());

        for (llvm::Value* value : values) {
          if (auto constant = llvm::dyn_cast<llvm::Constant>(value))
            constants.push_back(constant);

          // non-const global arrays
          else {
            logger.log(Logger::Level::ERROR, "non-const global arrays are not allowed");
            return {};
          }
        }

        return {llvm::ConstantArray::get(arr_type, constants), arr_type};
      }

      llvm::AllocaInst* arr = builder->CreateAlloca(arr_type);

      for (size_t i = 0; i < values.size(); ++i) {
        llvm::Value* element_ptr = builder->CreateConstInBoundsGEP2_32(arr_type, arr, 0, i);
        builder->CreateStore(values[i], element_ptr);
      }

      return {arr, arr_type};
    }
    Data Visitor::lvalue(ArrLitExpr* expr) { return {}; }

    Data Visitor::rvalue(IdeExpr* expr) {
      if (named_variables.find(expr->name) == named_variables.end())
        logger.log(Logger::Level::ERROR, "Use of undeclared identifier '" + expr->name + "'", true);

      Data* variable = &named_variables[expr->name];
      llvm::Value* value = variable->value;
      llvm::Type* type = variable->type;

      if (type->isArrayTy())
        return {value, type, variable};

      return {builder->CreateLoad(type, value), type, variable};
    }
    Data Visitor::lvalue(IdeExpr* expr) {
      if (named_variables.find(expr->name) == named_variables.end())
        logger.log(Logger::Level::ERROR, "Use of undeclared identifier '" + expr->name + "'", true);

      Data* variable = &named_variables[expr->name];
      llvm::Value* value = variable->value;
      llvm::Type* type = variable->type;

      // don't load
      return {value, type, variable};
    }
  
    Data Visitor::rvalue(BinOpExpr* expr) {
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
    Data Visitor::lvalue(BinOpExpr* expr) { return {}; }

    Data Visitor::rvalue(RefExpr* expr) {
      Data* variable = &named_variables[expr->ide->name];
      llvm::Value* ptr = variable->value;
      llvm::Type* type = variable->type;

      if (!ptr)
        logger.log(Logger::Level::ERROR, "Use of undeclared identifier '" + expr->ide->name + "'", true);

      return {ptr, ptr->getType(), variable};
    }
    Data Visitor::lvalue(RefExpr* expr) { return {}; }

    Data Visitor::rvalue(DeRefExpr* expr) {
      Data pointer_expr = expr->ptr_expr->rvalue(this);

      // identifier dereference: *x
      if (pointer_expr.variable) {
        if (!pointer_expr.variable->ptr_to_variable)
          logger.log(Logger::Level::ERROR, "Trying to dereference a non-pointer type '" + pointer_expr.variable->name + "'", true);

        Data* variable = pointer_expr.variable->ptr_to_variable;

        return {builder->CreateLoad(variable->type, pointer_expr.value), variable->type, variable};
      }

      // binary operator dereference: *(x + i) || x[i]
      return {builder->CreateLoad(pointer_expr.type, pointer_expr.value), pointer_expr.type};
    }
    Data Visitor::lvalue(DeRefExpr* expr) {
      Data pointer_expr = expr->ptr_expr->rvalue(this);

      if (pointer_expr.variable) {
        if (!pointer_expr.variable->ptr_to_variable)
          logger.log(Logger::Level::ERROR, "Trying to dereference a non-pointer type '" + pointer_expr.variable->name + "'", true);

        return {pointer_expr.value, pointer_expr.type, pointer_expr.variable->ptr_to_variable};
      }

      return {pointer_expr.value, pointer_expr.type};

      // Data pointer_expr = expr->ptr_expr->lvalue(this);
      //
      // if (!pointer_expr.variable)
      //   logger.log(Logger::Level::ERROR, "Dereferencing operations only supports direct identifiers", true);
      //
      // if (!pointer_expr.variable->ptr_to_variable)
      //   logger.log(Logger::Level::ERROR, "Trying to dereference a non-pointer type '" + pointer_expr.variable->name + "'", true);
      //
      // Variable* next_variable = pointer_expr.variable->ptr_to_variable;
      //
      // return {builder->CreateLoad(pointer_expr.type, pointer_expr.value), pointer_expr.type, next_variable};
    }

    Data Visitor::rvalue(VarDecExpr* expr) {
      if (this->inside_function)
        return create_local_variable(expr);
      else
        return create_global_variable(expr);
    }
    Data Visitor::lvalue(VarDecExpr* expr) { return {}; }

    Data Visitor::rvalue(ArrDecExpr* expr) {
      if (this->inside_function)
        return create_local_array(expr);
      else
        return create_global_array(expr);
    }
    Data Visitor::lvalue(ArrDecExpr* expr) { return {}; }

    Data Visitor::rvalue(FnCallExpr* expr) {
      llvm::Function* calle_fn = module->getFunction(expr->name);

      if (!calle_fn)
        logger.log(Logger::Level::ERROR, "undefined reference to function call: '" + expr->name + "'", true);

      if (calle_fn->arg_size() != expr->args.size())
        logger.log(Logger::Level::ERROR, "Incorrect signature for function call: '" + expr->name + "'", true);

      std::vector<llvm::Value*> args_values;
      args_values.reserve(expr->args.size());

      for (size_t i = 0; i < expr->args.size(); ++i) {
        Data arg_expr = expr->args[i]->rvalue(this);

        if (!arg_expr.value)
          logger.log(Logger::Level::ERROR, "Unidentified argument value for function '" + expr->name + "'", true);

        args_values.push_back(arg_expr.value);
      }

      return {builder->CreateCall(calle_fn, args_values, expr->name), calle_fn->getReturnType()};
    }
    Data Visitor::lvalue(FnCallExpr* expr) { return {}; }

    Data Visitor::visit(ReturnStt* stt) {
      llvm::BasicBlock* current_block = builder->GetInsertBlock();

      if (!current_block)
        logger.log(Logger::Level::ERROR, "Incorrect return statement place", true);

      llvm::Function* current_function = current_block->getParent();

      if (!current_function)
        logger.log(Logger::Level::ERROR, "return outside of a function is not allowed", true);

      if (!stt->expr)
        return {builder->CreateRetVoid(), llvm::Type::getVoidTy(*context)};

      // return value
      Data return_expr = stt->expr->rvalue(this);

      if (!return_expr.value)
        logger.log(Logger::Level::ERROR, "Unexpected error in return statement for function '" + std::string(current_function->getName()) + "'", true);

      // complete here
      llvm::Value* return_value = return_expr.value;

      if (!return_value)
        return nullptr;

      // current function return type
      llvm::Type* fn_return_type = current_function->getReturnType();

      if (fn_return_type != return_value->getType())
        return_value = operation->cast(return_value, fn_return_type);

      if (!return_value) {
        logger.log(Logger::Level::WARNING, "Unexpected return type for function '" + current_function->getName().str() + "', going back to void return");
        return builder->CreateRetVoid();
      }

      return builder->CreateRet(return_value);
    }

    Data Visitor::visit(ExprStt* stt) {
      return stt->expr->rvalue(this);
    }

    Data Visitor::visit(FnDecStt* stt) {
      // Convert parameter types
      std::vector<llvm::Type*> param_types;
      param_types.reserve(stt->params.size());

      for (auto& param : stt->params) {
        Data param_info = param->initializer->rvalue(this);

        param->variable.type = param_info.type;
        llvm::Type* param_type = param->variable.type;

        if (!param_type)
          logger.log(Logger::Level::ERROR, "Unrecognized data type '" + Help::llvm_to_string(param_type, logger) + "': '" + param->variable.name + "'", true);

        param_types.push_back(param_type);
      }

      // Get return type
      llvm::Type* return_type = Help::string_to_llvm(stt->type, context, logger);

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

    Data Visitor::visit(FnDefStt* stt) {
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
      this->inside_function = true;

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
      this->inside_function = false;

      // back to the previous scope
      builder->ClearInsertionPoint();

      // Verify the function
      if (llvm::verifyFunction(*fn, &llvm::errs())) {
        logger.log(Logger::Level::ERROR, "Function verification failed for '" + stt->declaration->type + "' : '" + stt->declaration->name + "'");

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
