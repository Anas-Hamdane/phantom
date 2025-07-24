#ifndef PHANTOM_TOKEN_HPP
#define PHANTOM_TOKEN_HPP

#include "info.hpp"
#include <string>

namespace phantom {
  struct Token {
    enum class Kind {
      // data types
      Bool,
      Char,
      Short,
      Int,
      Long,
      Half,
      Float,
      Double,

      // keywords
      Return,
      Let,
      Fn,
      If,
      Else,
      While,
      For,

      // identifiers
      Identifier,

      // ponctuation
      Colon,
      SemiColon,
      Comma,
      OpenCurly,
      CloseCurly,
      OpenParent,
      CloseParent,
      OpenBracket,
      CloseBracket,
      Not,
      Mul,
      Div,
      Mod,
      And,
      Or,
      Plus,
      Minus,
      Inc,
      Dec,
      Less,
      LessEq,
      Greater,
      GreaterEq,
      Eq,
      EqEq,
      NotEq,
      Shl,
      ShlEq,
      Shr,
      ShrEq,
      ModEq,
      OrEq,
      AndEq,
      PlusEq,
      MinusEq,
      MulEq,
      DivEq,
      Qst,

      // literals
      IntLit,
      FloatLit,
      CharLit,
      StrLit,

      // enf of file flag
      EndOfFile,

      // Invalid Tokens
      Mongolien,
    };
    Kind kind;
    std::string form;
    Location location;

    Token(const Kind& kind, const Location& location)
        : kind(kind), location(location) {}

    Token(const Kind& kind, const std::string& form, const Location& location)
        : kind(kind), form(form), location(location) {}

    // clang-format off
    static std::string kind_to_string(Kind kind) {
      switch (kind) {
        // data types
        case Kind::Bool:   return "Bool";
        case Kind::Char:   return "Char";
        case Kind::Short:  return "Short";
        case Kind::Int:    return "Int";
        case Kind::Long:   return "Long";
        case Kind::Half:   return "Half";
        case Kind::Float:  return "Float";
        case Kind::Double: return "Double";
    
        // keywords
        case Kind::Return: return "Return";
        case Kind::Let:    return "Let";
        case Kind::Fn:     return "Fn";
        case Kind::If:     return "If";
        case Kind::Else:   return "Else";
        case Kind::While:  return "While";
        case Kind::For:    return "For";
    
        // identifiers
        case Kind::Identifier: return "Identifier";
    
        // punctuation
        case Kind::Colon:        return "Colon";
        case Kind::SemiColon:    return "SemiColon";
        case Kind::Comma:        return "Comma";
        case Kind::OpenCurly:    return "OpenCurly";
        case Kind::CloseCurly:   return "CloseCurly";
        case Kind::OpenParent:   return "OpenParent";
        case Kind::CloseParent:  return "CloseParent";
        case Kind::OpenBracket:  return "OpenBracket";
        case Kind::CloseBracket: return "CloseBracket";
        case Kind::Not:          return "Not";
        case Kind::Mul:          return "Mul";
        case Kind::Div:          return "Div";
        case Kind::Mod:          return "Mod";
        case Kind::And:          return "And";
        case Kind::Or:           return "Or";
        case Kind::Plus:         return "Plus";
        case Kind::Minus:        return "Minus";
        case Kind::Inc:          return "Inc";
        case Kind::Dec:          return "Dec";
        case Kind::Less:         return "Less";
        case Kind::LessEq:       return "LessEq";
        case Kind::Greater:      return "Greater";
        case Kind::GreaterEq:    return "GreaterEq";
        case Kind::Eq:           return "Eq";
        case Kind::EqEq:         return "EqEq";
        case Kind::NotEq:        return "NotEq";
        case Kind::Shl:          return "Shl";
        case Kind::ShlEq:        return "ShlEq";
        case Kind::Shr:          return "Shr";
        case Kind::ShrEq:        return "ShrEq";
        case Kind::ModEq:        return "ModEq";
        case Kind::OrEq:         return "OrEq";
        case Kind::AndEq:        return "AndEq";
        case Kind::PlusEq:       return "PlusEq";
        case Kind::MinusEq:      return "MinusEq";
        case Kind::MulEq:        return "MulEq";
        case Kind::DivEq:        return "DivEq";
        case Kind::Qst:          return "Qst";
    
        // literals
        case Kind::IntLit:   return "IntLit";
        case Kind::FloatLit: return "FloatLit";
        case Kind::CharLit:  return "CharLit";
        case Kind::StrLit:   return "StrLit";
    
        // end of file
        case Kind::EndOfFile: return "EndOfFile";
    
        // invalid tokens
        case Kind::Mongolien: return "Mongolien";
      }
    }
    // clang-format on
  };
} // namespace phantom

#endif // !PHANTOM_TOKEN_HPP
