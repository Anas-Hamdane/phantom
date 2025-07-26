#include <ast/Expr.hpp>

namespace phantom {
  AstElm DataTypeExpr::represent() const {
    if (length)
      return AstElm{"DataTypeExpr: " + type + "\n", {length->represent()}};

    return AstElm{"DataTypeExpr: " + type + "\n", {}};
  }
  AstElm IntLitExpr::represent() const {
    return AstElm{"IntLitExpr: " + std::to_string(value) + "\n", {}};
  }
  AstElm FloatLitExpr::represent() const {
    return AstElm{"FloatLitExpr: " + std::to_string(value) + "\n", {}};
  }
  AstElm CharLitExpr::represent() const {
    return AstElm{"CharLitExpr: " + std::string(1, value) + "\n", {}};
  }
  AstElm BoolLitExpr::represent() const {
    return AstElm{"BoolLitExpr: " + std::string((value == true) ? "true" : "false") + "\n", {}};
  }
  AstElm StrLitExpr::represent() const {
    return AstElm{"StrLitExpr: " + value + "\n", {}};
  }
  AstElm ArrLitExpr::represent() const {
    std::vector<AstElm> hmm;

    for (auto& elem : elements)
      hmm.push_back(elem->represent());

    return AstElm{"ArrLitExpr:\n", hmm};
  }
  AstElm IdeExpr::represent() const {
    return AstElm{"IdeExpr: " + name + "\n", {}};
  }
  AstElm BinOpExpr::represent() const {
    return AstElm{"BinOpExpr: " + Token::kind_to_string(op) + "\n", {}};
  }
  AstElm VarDecExpr::represent() const {

    if (value && type)
      return AstElm{"VarDecExpr: " + name + "\n", {value->represent(), type->represent()}};

    if (value)
      return AstElm{"VarDecExpr: " + name + "\n", {value->represent()}};

    if (type)
      return AstElm{"VarDecExpr: " + name + "\n", {type->represent()}};

    return AstElm{"VarDecExpr: " + name + "\n", {}};
  }
  AstElm UnaryExpr::represent() const {
    return AstElm{"UnaryExpr: " + Token::kind_to_string(op) + "\n", {expr->represent()}};
  }
  AstElm FnCallExpr::represent() const {
    std::vector<AstElm> elms;

    for (auto& elm : args)
      elms.push_back(elm->represent());

    return AstElm{"FnCallExpr: " + name + "\n", elms};
  }
} // namespace phantom
