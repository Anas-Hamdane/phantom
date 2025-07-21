#include <ast/Expression.hpp>

namespace phantom {
  Value TypeExpr::gen(Visitor* visitor) { return visitor->gen(this); }

  Value IntLitExpr::gen(Visitor* visitor) { return visitor->gen(this); }
  Value FloatLitExpr::gen(Visitor* visitor) { return visitor->gen(this); }
  Value CharLitExpr::gen(Visitor* visitor) { return visitor->gen(this); }
  Value BoolLitExpr::gen(Visitor* visitor) { return visitor->gen(this); }
  Value StrLitExpr::gen(Visitor* visitor) { return visitor->gen(this); }
  Value ArrLitExpr::gen(Visitor* visitor) { return visitor->gen(this); }

  Value IdeExpr::gen(Visitor* visitor) { return visitor->gen(this); }
  Value RefExpr::gen(Visitor* visitor) { return visitor->gen(this); }
  Value DeRefExpr::gen(Visitor* visitor) { return visitor->gen(this); }

  Value VarDecExpr::gen(Visitor* visitor) { return visitor->gen(this); }
  Value BinOpExpr::gen(Visitor* visitor) { return visitor->gen(this); }
  Value FnCallExpr::gen(Visitor* visitor) { return visitor->gen(this); }
} // namespace phantom
