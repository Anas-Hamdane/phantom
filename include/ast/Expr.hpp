#pragma once

#include <Token.hpp>
#include <common.hpp>
#include <utils/vec.hpp>

namespace phantom {
  struct Expr;
  enum class ExprKind : uint {
    Invalid = 0,
    DataType,
    BoolLit,
    CharLit,
    IntLit,
    FloatLit,
    StrLit,
    ArrLit,
    Identifier,
    BinOp,
    UnOp,
    VarDecl,
    Param,
    FnCall
  };

  struct Invalid {};
  struct DataType {
    Type type;
    Expr* length;
  };
  struct BoolLit {
    bool value;
  };
  struct CharLit {
    char value;
  };
  struct IntLit {
    const char* form;
    uint64_t value;
  };
  struct FloatLit {
    const char* form;
    double value;
  };
  struct StrLit {
    const char* value;
  };
  struct ArrLit {
    utils::Vec<Expr> elements;
  };
  struct Identifier {
    const char* name;
  };
  struct BinOp {
    Expr* lhs;
    Token::Kind op;
    Expr* rhs;
  };
  struct UnOp {
    Expr* operand;
    Token::Kind op;
  };
  struct VarDecl {
    Identifier* ide;
    DataType* type;
    Expr* value;
  };
  struct Param {
    Identifier* ide;
    DataType* type;
  };
  struct FnCall {
    const char* name;
    utils::Vec<Expr> args;
  };

  struct Expr {
    ExprKind kind;

    union {
      Invalid invalid;
      DataType data_type;
      BoolLit bool_lit;
      CharLit char_lit;
      IntLit int_lit;
      FloatLit float_lit;
      StrLit str_lit;
      ArrLit arr_lit;
      Identifier ide;
      BinOp binop;
      UnOp unop;
      VarDecl var_decl;
      Param param;
      FnCall fn_call;
    } data;
  };

} // namespace phantom
