#include <ast/Statement.hpp>
#include <llvm_codegen/Help.hpp>
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

    llvm::Value* Visitor::make_global_variable(VarDecExpr* expr) {
      std::string name = expr->name;
      llvm::Constant* value = nullptr;

      if (name_table.find(name) != name_table.end())
        logger.log(Logger::Level::WARNING, "Redefinition of an already existing variable '" + name + "'");

      if (!expr->initializer) {
        logger.log(Logger::Level::ERROR, "Could not initialize type for variable '" + name + "'");
        return {};
      }

      // initializer available
      llvm::Value* init = expr->initializer->gen(this);

      if (!init) {
        logger.log(Logger::Level::FATAL, "could not get the value of the variable'" + name + "'", true);
        return nullptr;
      }

      if (!(value = llvm::dyn_cast<llvm::Constant>(init))) {
        logger.log(Logger::Level::ERROR, "Global variables must have a constant initializer, '" + name + "'");
        return nullptr;
      }

      // declare the global variable
      llvm::GlobalVariable* global_variable = new llvm::GlobalVariable(
          *module,                            // module
          value->getType(),                   // type
          false,                              // constant
          llvm::GlobalValue::ExternalLinkage, // linkage
          value,                              // Constant
          name                                // name
      );

      TypeInfo::Kind kind = Help::llvm_to_kind(value->getType(), logger);
      TypeInfo::Kind element_kind = TypeInfo::Kind::BAKA;
      size_t array_size = 0;

      if (kind == TypeInfo::Kind::Array) {
        array_size = value->getType()->getArrayNumElements();
        element_kind = Help::llvm_to_kind(value->getType()->getArrayElementType(), logger);
      }

      auto element_type = element_kind == TypeInfo::Kind::BAKA ? std::make_shared<TypeInfo>(element_kind) : nullptr;
      VarInfo variable(name, global_variable, std::make_shared<TypeInfo>(kind, element_type, array_size));

      name_table[name] = variable;
      value_table[global_variable] = variable;
      return global_variable;
    }
    llvm::Value* Visitor::make_local_variable(VarDecExpr* expr) {
      std::string name = expr->name;
      llvm::Value* value = nullptr;
      llvm::Type* type = nullptr;

      if (name_table.find(name) != name_table.end())
        logger.log(Logger::Level::WARNING, "Redefinition of an already existing variable '" + name + "'");

      if (!expr->initializer) {
        logger.log(Logger::Level::ERROR, "Could not initialize type for variable '" + name + "'");
        return nullptr;
      }

      // initializer available
      llvm::Value* init = expr->initializer->gen(this);

      if (!init) {
        logger.log(Logger::Level::FATAL, "not enough informations about variable '" + name + "'", true);
        return nullptr;
      }

      llvm::AllocaInst* var = builder->CreateAlloca(value->getType());
      builder->CreateStore(value, var);

      DataType data(name, var, type);

      data_table[name] = data;
      value_table[name] = var;
      return {var, Value::Type::Alloca, &data_table[name]};
    }

    llvm::Value* Visitor::make_global_array(std::vector<std::unique_ptr<Expr>> elements) {
      llvm::ArrayType* array_type = nullptr;

      std::vector<llvm::Constant*> constants;
      constants.reserve(elements.size());

      llvm::Value* reference = elements[0]->gen(this);

      if (auto constant = llvm::dyn_cast<llvm::Constant>(reference.value))
        constants.push_back(constant);
      else
        goto fail;

      for (size_t i = 1; i < elements.size(); i++) {
        llvm::Value* elem = elements[i]->gen(this);

        if (reference.value->getType() != elem.value->getType())
          goto fail;

        if (auto constant = llvm::dyn_cast<llvm::Constant>(elem.value))
          constants.push_back(constant);
        else
          goto fail;
      }

      array_type = llvm::ArrayType::get(reference.value->getType(), constants.size());
      return {llvm::ConstantArray::get(array_type, constants), Value::Type::Constant};

    fail:
      logger.log(Logger::Level::ERROR, "Global arrays must have constants elements with the same type");
      return {};
    }
    llvm::Value* Visitor::make_local_array(std::vector<std::unique_ptr<Expr>> elements) {
      llvm::ArrayType* array_type = nullptr;
      llvm::AllocaInst* array = nullptr;

      std::vector<llvm::Value*> values;
      values.reserve(elements.size());

      llvm::Value* reference = elements[0]->gen(this);
      values.push_back(reference.value);

      for (size_t i = 1; i < elements.size(); ++i) {
        llvm::Value* elem = elements[i]->gen(this);

        if (reference.value->getType() != elem.value->getType())
          goto fail;

        values.push_back(elem.value);
      }

      array_type = llvm::ArrayType::get(reference.value->getType(), values.size());
      array = builder->CreateAlloca(array_type);

      for (size_t i = 0; i < values.size(); ++i) {
        llvm::Value* index = llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context), i);

        llvm::Value* element_ptr = builder->CreateGEP(
            values[i]->getType(),
            array,
            {index});

        builder->CreateStore(values[i], element_ptr);
      }

      return {array, Value::Type::Alloca};

    fail:
      logger.log(Logger::Level::ERROR, "Arrays elements must have the same type");
      return {};
    }

    llvm::Value* Visitor::gen(TypeExpr* expr, GenMode mode) {
      if (!expr->length) {
        llvm::Type* type = Help::string_to_llvm(expr->type, context, logger);
        llvm::Value* value = llvm::Constant::getNullValue(type);
        auto value_type = Value::Type::Constant;

        if (expr->value) {
          auto expr_result = expr->value->gen(this);
          value = expr_result.value;
          value_type = expr_result.type;

          if (value->getType() != type)
            value = operation->cast(value, type);

          if (!value) {
            logger.log(Logger::Level::ERROR, "Incompatible types: " + Help::llvm_to_string(value->getType(), logger) + "', '" + Help::llvm_to_string(type, logger) + "'");
            return {};
          }
        }

        return value;
      }

      // TODO: arrays
    }

    llvm::Value* Visitor::gen(IntLitExpr* expr) {
      llvm::Type* type = nullptr;

      if (expr->value >= INT_MIN_VAL && expr->value <= INT_MAX_VAL)
        type = llvm::Type::getInt32Ty(*context);

      else if (expr->value >= LONG_MIN_VAL && expr->value <= LONG_MAX_VAL)
        type = llvm::Type::getInt64Ty(*context);

      else
        logger.log(Logger::Level::ERROR, "llvm::Value* can't be represented in a data type");

      return {llvm::ConstantInt::get(type, expr->value), Value::Type::Constant};
    }
    llvm::Value* Visitor::gen(FloatLitExpr* expr) {
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
        logger.log(Logger::Level::ERROR, "llvm::Value* can't be represented in a data type", true);

      return {llvm::ConstantFP::get(type, expr->value), Value::Type::Constant};
    }
    llvm::Value* Visitor::gen(CharLitExpr* expr) {
      llvm::Type* type = llvm::Type::getInt8Ty(*context);
      return {llvm::ConstantInt::get(type, expr->value), Value::Type::Constant};
    }
    llvm::Value* Visitor::gen(BoolLitExpr* expr) {
      llvm::Type* type = llvm::Type::getInt1Ty(*context);
      return {llvm::ConstantInt::get(type, expr->value), Value::Type::Constant};
    }
    llvm::Value* Visitor::gen(StrLitExpr* expr) {
      return {builder->CreateGlobalString(expr->value), Value::Type::Constant};
    }
    llvm::Value* Visitor::gen(ArrLitExpr* expr) {
      if (expr->elements.size() == 0) {
        logger.log(Logger::Level::ERROR, "Empty array literals are not allowed");
        return {};
      }

      if (!this->inside_function)
        return make_global_array(std::move(expr->elements));
      else
        return make_local_array(std::move(expr->elements));
    }

    llvm::Value* Visitor::gen(IdeExpr* expr) {
      if (data_table.find(expr->name) == data_table.end()) {
        logger.log(Logger::Level::ERROR, "Use of undeclared identifier '" + expr->name + "'");
        return {};
      }

      return {data_table[expr->name].value, Value::Type::Alloca, &data_table[expr->name]};
    }
    llvm::Value* Visitor::gen(RefExpr* expr) {
      if (expr->ide->expr_type() != ExprType::Ide) {
        logger.log(Logger::Level::ERROR, "Expected identifier after '&' operator");
        return {};
      }

      return expr->ide->gen(this);
    }
    llvm::Value* Visitor::gen(DeRefExpr* expr) {
    }

    llvm::Value* Visitor::gen(BinOpExpr* expr) {
      if (expr->op == TokenType::EQUAL)
        return operation->asgn(expr->left->gen(this), expr->right->gen(this));

      switch (expr->op) {
        case TokenType::PLUS:
          return operation->add(expr->left->gen(this), expr->right->gen(this));
        case TokenType::MINUS:
          return operation->sub(expr->left->gen(this), expr->right->gen(this));
        case TokenType::STAR:
          return operation->mul(expr->left->gen(this), expr->right->gen(this));
        case TokenType::SLASH:
          return operation->div(expr->left->gen(this), expr->right->gen(this));
        default:
          logger.log(Logger::Level::ERROR, "invalid binary operator", true);
      }

      return {};
    }
    llvm::Value* Visitor::gen(VarDecExpr* expr) {
      if (!this->inside_function)
        return make_global_variable(expr);
      else
        return make_local_variable(expr);
    }
    llvm::Value* Visitor::gen(FnCallExpr* expr) {
      llvm::Function* callee_fn = module->getFunction(expr->name);

      if (!callee_fn) {
        logger.log(Logger::Level::ERROR, "undefined reference to function call: '" + expr->name + "'");
        return {};
      }

      if (callee_fn->arg_size() != expr->args.size()) {
        logger.log(Logger::Level::ERROR, "Incorrect signature for function call: '" + expr->name + "'", true);
        return {};
      }

      std::vector<llvm::Value*> args_values;
      args_values.reserve(expr->args.size());

      for (size_t i = 0; i < expr->args.size(); ++i) {
        llvm::Value* arg_expr = expr->args[i]->gen(this);

        if (!arg_expr.value) {
          logger.log(Logger::Level::ERROR, "Unidentified argument value for function '" + expr->name + "'", true);
          return {};
        }

        args_values.push_back(arg_expr.value);
      }

      return {builder->CreateCall(callee_fn, args_values), Value::Type::Operation};
    }

    llvm::Value* Visitor::gen(ReturnStt* stt) {
      llvm::BasicBlock* current_block = builder->GetInsertBlock();

      if (!current_block) {
        logger.log(Logger::Level::ERROR, "Incorrect return statement place");
        return {};
      }

      llvm::Function* current_function = current_block->getParent();

      if (!current_function) {
        logger.log(Logger::Level::ERROR, "return outside of a function is not allowed", true);
        return {};
      }

      if (!stt->expr)
        return {builder->CreateRetVoid(), Value::Type::Terminator};

      // return value
      llvm::Value* return_expr = stt->expr->gen(this);

      if (!return_expr.value) {
        logger.log(Logger::Level::ERROR, "Unexpected error in return statement for function '" + std::string(current_function->getName()) + "'");
        return {};
      }

      llvm::Value* return_value = operation->resolve_value(return_expr);
      llvm::Type* fn_return_type = current_function->getReturnType();

      if (fn_return_type != return_value->getType())
        return_value = operation->cast(return_value, fn_return_type);

      if (!return_value) {
        logger.log(Logger::Level::WARNING, "Unexpected return type for function '" + current_function->getName().str() + "', going back to default return");
        return {Help::default_return(fn_return_type, builder), Value::Type::Terminator};
      }

      return {builder->CreateRet(return_value), Value::Type::Terminator};
    }

    llvm::Value* Visitor::gen(ExprStt* stt) {
      return stt->expr->gen(this);
    }

    llvm::Value* Visitor::gen(FnDecStt* stt) {
      // Convert parameter types
      std::vector<llvm::Type*> params_types;
      params_types.reserve(stt->params.size());

      for (auto& param : stt->params) {
        llvm::Value* param_info = param->initializer->gen(this);

        llvm::Type* param_type = param_info.value->getType();

        if (!param_type) {
          logger.log(Logger::Level::ERROR, "Unrecognized type for variable '" + param->name + "'");
          return {};
        }

        params_types.push_back(param_type);
      }

      // Get return type
      llvm::Type* return_type = Help::string_to_llvm(stt->type, context, logger);

      if (!return_type) {
        logger.log(Logger::Level::ERROR, "Unrecognized return type for variable '" + stt->name + "' with a return type of '" + stt->type + "'");
        return {};
      }

      // Create function type
      llvm::FunctionType* fn_type = llvm::FunctionType::get(
          return_type,  // return type
          params_types, // prameters types
          false         // variable argument
      );

      // Create function
      llvm::Function* fn = llvm::Function::Create(
          fn_type,
          llvm::Function::ExternalLinkage,
          0,
          stt->name,
          module.get());

      return {fn, Value::Type::Function};
    }

    llvm::Value* Visitor::gen(FnDefStt* stt) {
      llvm::Function* fn = module->getFunction(stt->declaration->name);

      if (!fn)
        fn = llvm::dyn_cast<llvm::Function>(stt->declaration->gen(this).value);

      if (!fn) {
        logger.log(Logger::Level::ERROR, "Could not declare function '" + stt->declaration->name + "' with type '" + stt->declaration->type + "'");
        return {};
      }

      // has no body
      if (!fn->empty()) {
        logger.log(Logger::Level::ERROR, "Function cannot be redefined '" + stt->declaration->name + "'");
        return {};
      }

      // Create basic block for function body
      llvm::BasicBlock* BB = llvm::BasicBlock::Create(*context, "entry", fn);
      builder->SetInsertPoint(BB);

      /*
       * NOTE: for nested scopes, save the named_values as "old_named_values"
       * add the arguments of the functions to the named_values, generate the code
       * for function body, restore the "old_named_values" in named_values.
       */
      auto old_data_table = data_table;
      auto old_value_table = value_table;
      this->inside_function = true;

      // arguments handling
      size_t idx = 0;
      for (auto& arg : fn->args()) {
        llvm::AllocaInst* alloca = builder->CreateAlloca(arg.getType());
        builder->CreateStore(&arg, alloca);

        std::string name = stt->declaration->params[idx]->name;
        data_table[name] = DataType(name, alloca, alloca->getAllocatedType());
        value_table[name] = alloca;
        ++idx;
      }

      for (auto& stmt : stt->body)
        stmt->gen(this);

      // adjust return value stuff
      llvm::BasicBlock* currentBlock = builder->GetInsertBlock();
      if (currentBlock && !currentBlock->getTerminator())
        Help::default_return(fn->getReturnType(), builder);

      // clear this scope's variables
      data_table = old_data_table;
      value_table = old_value_table;
      this->inside_function = false;

      // back to the previous scope
      builder->ClearInsertionPoint();

      // Verify the function
      if (llvm::verifyFunction(*fn, &llvm::errs())) {
        logger.log(Logger::Level::ERROR, "\nFunction verification failed for '" + stt->declaration->type + "' : '" + stt->declaration->name + "'");

        fn->eraseFromParent();
        return {};
      }

      // optimize the function
      if (FPM && FAM)
        FPM->run(*fn, *FAM);

      return {fn, Value::Type::Function};
    }
  } // namespace llvm_codegen
} // namespace phantom
