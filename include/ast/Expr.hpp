#pragma once

#include <Token.hpp>
#include <common.hpp>

namespace phantom {
  struct Expr;
  enum class ExprKind {
    Invalid,
    DataType,
    IntLit,
    FloatLit,
    CharLit,
    BoolLit,
    StrLit,
    ArrLit,
    Identifier,
    BinOp,
    VarDecl,
    UnOp,
    FnCall
  };

  struct Invalid {};
  struct DataType {
    const char* type;
    ExprRef length;
  };
  struct IntLit {
    const char* form;
    uint64_t value;
  };
  struct FloatLit {
    const char* form;
    long double value;
  };
  struct CharLit {
    char value;
  };
  struct BoolLit {
    bool value;
  };
  struct StrLit {
    const char* value;
  };
  struct ArrLit {
    ExprRef* elems;
    size_t len;
  };
  struct Identifier {
    const char* name;
  };
  struct BinOp {
    ExprRef left;
    Token::Kind op;
    ExprRef right;
  };
  struct UnOp {
    ExprRef expr;
    Token::Kind op;
  };
  struct VarDecl {
    const char* name;
    ExprRef value;
    ExprRef type;
  };
  struct FnCall {
    const char* name;
    ExprRef* args;
    size_t len;
  };

  struct Expr {
    ExprKind kind;

    union {
      Invalid invalid;
      DataType data_type;     
      IntLit int_lit;
      FloatLit float_lit;
      CharLit char_lit;
      BoolLit bool_lit;
      StrLit str_lit;
      ArrLit arr_lit;
      Identifier ide;
      BinOp binop;
      VarDecl var_decl;
      UnOp unop;
      FnCall fn_call;
    } data;
  };
} // namespace phantom
