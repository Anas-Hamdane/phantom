#pragma once

#include <Token.hpp>
#include <common.hpp>
#include <memory>
#include <variant>

namespace phantom {
  namespace ast {
    struct IntLit;
    struct FloatLit;
    struct StrLit;
    struct ArrLit;
    struct Identifier;
    struct BinOp;
    struct UnOp;
    struct VarDecl;
    struct FnCall;

    using Expr = std::variant<std::unique_ptr<IntLit>, std::unique_ptr<FloatLit>, std::unique_ptr<StrLit>,
                              std::unique_ptr<ArrLit>, std::unique_ptr<Identifier>, std::unique_ptr<BinOp>,
                              std::unique_ptr<UnOp>, std::unique_ptr<VarDecl>, std::unique_ptr<FnCall>>;

    struct IntLit {
      uint64_t value;
    };
    struct FloatLit {
      double value;
    };
    struct StrLit {
      std::string value;
    };
    struct ArrLit {
      std::vector<std::unique_ptr<Expr>> elements;
    };
    struct Identifier {
      std::string name;
    };
    struct BinOp {
      std::unique_ptr<Expr> lhs;
      Token::Kind op;
      std::unique_ptr<Expr> rhs;
    };
    struct UnOp {
      std::unique_ptr<Expr> operand;
      Token::Kind op;
    };
    struct VarDecl {
      std::string name;
      std::unique_ptr<Type> type;
      std::unique_ptr<Expr> init;
    };
    struct FnCall {
      std::string name;
      std::vector<std::unique_ptr<Expr>> args;
    };
  } // namespace ast
} // namespace phantom
