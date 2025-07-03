#ifndef PHANTOM_EXPRESSIONS_HPP
#define PHANTOM_EXPRESSIONS_HPP

#include <Lexer/Token.hpp>
#include <global.hpp>

namespace phantom {
  class Visitor;
  class Expression {
public:
    virtual ~Expression();
    virtual llvm::Value* accept(Visitor* visitor) = 0;
  };

  class IntLitExpr : public Expression {
public:
    std::string form;
    long long value; // always 8 bytes

    explicit IntLitExpr(std::string form);
    llvm::Value* accept(Visitor* visitor) override;
  };

  class FloatLitExpr : public Expression {
public:
    std::string form;
    long double value; // largest possible

    explicit FloatLitExpr(std::string form);
    llvm::Value* accept(Visitor* visitor) override;
  };

  class CharLitExpr : public Expression {
public:
    char value;

    explicit CharLitExpr(char value);
    llvm::Value* accept(Visitor* visitor) override;
  };

  class BoolLitExpr : public Expression {
public:
    std::string form;
    bool value;

    explicit BoolLitExpr(std::string form);
    llvm::Value* accept(Visitor* visitor) override;
  };

  class StrLitExpr : public Expression {
public:
    std::string value;

    explicit StrLitExpr(std::string value);
    llvm::Value* accept(Visitor* visitor) override;
  };

  class IdentifierExpr : public Expression {
public:
    std::string name;
    bool positive;

    explicit IdentifierExpr(std::string name, bool positive = true);
    llvm::Value* accept(Visitor* visitor) override;
  };

  class BinOpExpr : public Expression {
public:
    std::unique_ptr<Expression> left;
    std::unique_ptr<Expression> right;
    TokenType op;

    BinOpExpr(std::unique_ptr<Expression> left, TokenType op, std::unique_ptr<Expression> right);
    llvm::Value* accept(Visitor* visitor) override;
  };

  class AddrExpr : public Expression {
public:
    std::string variable;

    AddrExpr(std::string variable);
    llvm::Value* accept(Visitor* visitor) override;
  };

  class FnCallExpr : public Expression {
public:
    std::string name;
    std::vector<std::unique_ptr<Expression>> args;

    explicit FnCallExpr(std::string name, std::vector<std::unique_ptr<Expression>> args);
    llvm::Value* accept(Visitor* visitor) override;
  };
} // namespace phantom

#endif // !PHANTOM_EXPRESSIONS_HPP
