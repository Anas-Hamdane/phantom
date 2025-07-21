#include <ast/Statement.hpp>

namespace phantom {
  Statement::~Statement() = default;

  Value ReturnStt::gen(Visitor* visitor) { return visitor->gen(this); }
  Value ExprStt::gen(Visitor* visitor) { return visitor->gen(this); }
  Value FnDefStt::gen(Visitor* visitor) { return visitor->gen(this); }
  Value FnDecStt::gen(Visitor* visitor) { return visitor->gen(this); }
} // namespace phantom
