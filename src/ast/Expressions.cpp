#include <ast/Expression.hpp>

namespace phantom {
  ExprInfo DataTypeExpr::rvalue(Visitor* visitor) { return visitor->rvalue(this); }
  ExprInfo DataTypeExpr::lvalue(Visitor* visitor) { return visitor->lvalue(this); }

  ExprInfo ArrTypeExpr::rvalue(Visitor* visitor) { return visitor->rvalue(this); }
  ExprInfo ArrTypeExpr::lvalue(Visitor* visitor) { return visitor->lvalue(this); }

  ExprInfo IntLitExpr::rvalue(Visitor* visitor) { return visitor->rvalue(this); }
  ExprInfo IntLitExpr::lvalue(Visitor* visitor) { return visitor->lvalue(this); }

  ExprInfo FloatLitExpr::rvalue(Visitor* visitor) { return visitor->rvalue(this); }
  ExprInfo FloatLitExpr::lvalue(Visitor* visitor) { return visitor->lvalue(this); }

  ExprInfo CharLitExpr::rvalue(Visitor* visitor) { return visitor->rvalue(this); }
  ExprInfo CharLitExpr::lvalue(Visitor* visitor) { return visitor->lvalue(this); }

  ExprInfo BoolLitExpr::rvalue(Visitor* visitor) { return visitor->rvalue(this); }
  ExprInfo BoolLitExpr::lvalue(Visitor* visitor) { return visitor->lvalue(this); }

  ExprInfo StrLitExpr::rvalue(Visitor* visitor) { return visitor->rvalue(this); }
  ExprInfo StrLitExpr::lvalue(Visitor* visitor) { return visitor->lvalue(this); }

  ExprInfo ArrLitExpr::rvalue(Visitor* visitor) { return visitor->rvalue(this); }
  ExprInfo ArrLitExpr::lvalue(Visitor* visitor) { return visitor->lvalue(this); }

  ExprInfo IdeExpr::rvalue(Visitor* visitor) { return visitor->rvalue(this); }
  ExprInfo IdeExpr::lvalue(Visitor* visitor) { return visitor->lvalue(this); }

  ExprInfo BinOpExpr::rvalue(Visitor* visitor) { return visitor->rvalue(this); }
  ExprInfo BinOpExpr::lvalue(Visitor* visitor) { return visitor->lvalue(this); }

  ExprInfo RefExpr::rvalue(Visitor* visitor) { return visitor->rvalue(this); }
  ExprInfo RefExpr::lvalue(Visitor* visitor) { return visitor->lvalue(this); }

  ExprInfo DeRefExpr::rvalue(Visitor* visitor) { return visitor->rvalue(this); }
  ExprInfo DeRefExpr::lvalue(Visitor* visitor) { return visitor->lvalue(this); }

  ExprInfo FnCallExpr::rvalue(Visitor* visitor) { return visitor->rvalue(this); }
  ExprInfo FnCallExpr::lvalue(Visitor* visitor) { return visitor->lvalue(this); }
} // namespace phantom
