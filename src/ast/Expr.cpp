#include <ast/Expr.hpp>

namespace phantom {
  AstElm DataTypeExpr::represent() {}
  AstElm IntLitExpr::represent() {}
  AstElm FloatLitExpr::represent() {}
  AstElm CharLitExpr::represent() {}
  AstElm BoolLitExpr::represent() {}
  AstElm StrLitExpr::represent() {}
  AstElm ArrLitExpr::represent() {}
  AstElm IdeExpr::represent() {}
  AstElm BinOpExpr::represent() {}
  AstElm VarDecExpr::represent() {}
  AstElm UnaryExpr::represent() {}
  AstElm FnCallExpr::represent() {}
} // namespace phantom
