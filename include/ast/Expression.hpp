#ifndef PHANTOM_EXPRESSION_HPP
#define PHANTOM_EXPRESSION_HPP

#include <Token.hpp>
#include <memory>
#include <string>
#include <vector>

#include <common.hpp>

namespace phantom {
  enum class ExprType {
    Type,

    IntLit,
    FloatLit,
    CharLit,
    BoolLit,
    StrLit,
    ArrLit,

    Ide,
    BinOp,
    Ref,
    DeRef,

    VarDec,
    FnCall,
  };

  class Expr {
public:
    virtual ~Expr() = default;
    virtual ExprType expr_type() const = 0;
    virtual Value gen(Visitor* visitor) = 0;
  };

  class TypeExpr : public Expr {
public:
    const std::string type;
    std::unique_ptr<Expr> value;

    // for arrays
    std::unique_ptr<Expr> length;

    explicit TypeExpr(const std::string& type, std::unique_ptr<Expr> value, std::unique_ptr<Expr> length = nullptr)
        : type(type), value(std::move(value)), length(std::move(length)) {}

    ExprType expr_type() const override { return ExprType::Type; };
    Value gen(Visitor* visitor) override;
  };

  class IntLitExpr : public Expr {
public:
    const std::string form;
    long long value;

    explicit IntLitExpr(const std::string& form) : form(form), value(std::stoll(form)) {}

    ExprType expr_type() const override { return ExprType::IntLit; };
    Value gen(Visitor* visitor) override;
  };

  class FloatLitExpr : public Expr {
public:
    const std::string form;
    long double value;

    explicit FloatLitExpr(const std::string& form) : form(form), value(std::stold(form)) {}

    ExprType expr_type() const override { return ExprType::FloatLit; };
    Value gen(Visitor* visitor) override;
  };

  class CharLitExpr : public Expr {
public:
    char value;

    explicit CharLitExpr(char value) : value(value) {}

    ExprType expr_type() const override { return ExprType::CharLit; };
    Value gen(Visitor* visitor) override;
  };

  class BoolLitExpr : public Expr {
public:
    const std::string form;
    bool value;

    explicit BoolLitExpr(const std::string& form) : form(form), value(form == "true") {}

    ExprType expr_type() const override { return ExprType::BoolLit; };
    Value gen(Visitor* visitor) override;
  };

  class StrLitExpr : public Expr {
public:
    const std::string value;

    explicit StrLitExpr(const std::string& value) : value(value) {}

    ExprType expr_type() const override { return ExprType::StrLit; };
    Value gen(Visitor* visitor) override;
  };

  class ArrLitExpr : public Expr {
public:
    std::vector<std::unique_ptr<Expr>> elements;

    explicit ArrLitExpr(std::vector<std::unique_ptr<Expr>> elements)
        : elements(std::move(elements)) {}

    ExprType expr_type() const override { return ExprType::ArrLit; };
    Value gen(Visitor* visitor) override;
  };

  class IdeExpr : public Expr {
public:
    const std::string name;

    explicit IdeExpr(const std::string& name) : name(name) {}

    ExprType expr_type() const override { return ExprType::Ide; };
    Value gen(Visitor* visitor) override;
  };

  class BinOpExpr : public Expr {
public:
    std::unique_ptr<Expr> left;
    std::unique_ptr<Expr> right;
    const TokenType op;

    explicit BinOpExpr(std::unique_ptr<Expr> left, const TokenType& op, std::unique_ptr<Expr> right)
        : left(std::move(left)), op(op), right(std::move(right)) {}

    ExprType expr_type() const override { return ExprType::BinOp; };
    Value gen(Visitor* visitor) override;
  };

  class RefExpr : public Expr {
public:
    std::unique_ptr<Expr> ide;

    explicit RefExpr(std::unique_ptr<Expr> ide) : ide(std::move(ide)) {}

    ExprType expr_type() const override { return ExprType::Ref; };
    Value gen(Visitor* visitor) override;
  };

  class DeRefExpr : public Expr {
public:
    std::unique_ptr<Expr> pointer;

    explicit DeRefExpr(std::unique_ptr<Expr> ptr_expr) : pointer(std::move(ptr_expr)) {}

    ExprType expr_type() const override { return ExprType::DeRef; };
    Value gen(Visitor* visitor) override;
  };

  /*
   * TODO:
   *   implement const/static stuff
   */

  class VarDecExpr : public Expr {
public:
    std::string name;
    std::unique_ptr<Expr> initializer;

    VarDecExpr(const std::string& name, std::unique_ptr<Expr> initializer)
        : name(name), initializer(std::move(initializer)) {}

    ExprType expr_type() const override { return ExprType::VarDec; };
    Value gen(Visitor* visitor) override;
  };

  class FnCallExpr : public Expr {
public:
    const std::string name;
    std::vector<std::unique_ptr<Expr>> args;

    explicit FnCallExpr(const std::string& name, std::vector<std::unique_ptr<Expr>> args)
        : name(name), args(std::move(args)) {}

    ExprType expr_type() const override { return ExprType::FnCall; };
    Value gen(Visitor* visitor) override;
  };
} // namespace phantom

#endif // !PHANTOM_EXPRESSION_HPP
