#pragma once

#include <Token.hpp>
#include <common.hpp>
#include <memory>

namespace phantom {
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

  struct Invalid;
  struct DataType;
  struct IntLit;
  struct FloatLit;
  struct CharLit;
  struct BoolLit;
  struct StrLit;
  struct ArrLit;
  struct Identifier;
  struct BinOp;
  struct UnOp;
  struct VarDecl;
  struct Param;
  struct FnCall;

  struct Expr {
    ExprKind kind;

    union {
      struct Invalid {};
      struct DataType {
        Type type;
        std::unique_ptr<Expr> length;
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
        long double value;
      };
      struct StrLit {
        const char* value;
      };
      struct ArrLit {
        std::vector<std::unique_ptr<Expr>> elements;
        size_t len;
      };
      struct Identifier {
        const char* name;
      };
      struct BinOp {
        std::unique_ptr<Expr> left;
        Token::Kind op;
        std::unique_ptr<Expr> right;
      };
      struct UnOp {
        std::unique_ptr<Expr> expr;
        Token::Kind op;
      };
      struct VarDecl {
        std::unique_ptr<Expr> ide;
        std::unique_ptr<Expr> value;
        std::unique_ptr<Expr> type;
      };
      struct Param {
        std::unique_ptr<Expr> ide;
        std::unique_ptr<Expr> type;
      };
      struct FnCall {
        const char* name;
        std::vector<std::unique_ptr<Expr>> args;
        size_t len;
      };

    } data;
  };

} // namespace phantom
