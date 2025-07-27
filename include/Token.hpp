#pragma once

#include "info.hpp"
#include <string>

namespace phantom {
  struct Token {
public:
    enum class Kind {
      DataType,
      Identifier,

      // keywords
      Return,
      Let,
      Fn,
      If,
      Else,
      While,
      For,

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
      RightArrow,

      // literals
      IntLit,
      FloatLit,
      CharLit,
      StrLit,
      ArrLit,

      // enf of file flag
      EndOfFile,

      // Invalid Tokens
      Invalid,
    };

public:
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
        case Kind::DataType:   return "DataType";
        case Kind::Identifier: return "Identifier";
    
        // keywords
        case Kind::Return: return "return";
        case Kind::Let:    return "let";
        case Kind::Fn:     return "fn";
        case Kind::If:     return "if";
        case Kind::Else:   return "else";
        case Kind::While:  return "while";
        case Kind::For:    return "for";
    
        // punctuation
        case Kind::Colon:        return ":";
        case Kind::SemiColon:    return ";";
        case Kind::Comma:        return ",";
        case Kind::OpenCurly:    return "{";
        case Kind::CloseCurly:   return "}";
        case Kind::OpenParent:   return "(";
        case Kind::CloseParent:  return ")";
        case Kind::OpenBracket:  return "[";
        case Kind::CloseBracket: return "]";
        case Kind::Not:          return "!";
        case Kind::Mul:          return "*";
        case Kind::Div:          return "/";
        case Kind::Mod:          return "%";
        case Kind::And:          return "&";
        case Kind::Or:           return "|";
        case Kind::Plus:         return "+";
        case Kind::Minus:        return "-";
        case Kind::Inc:          return "++";
        case Kind::Dec:          return "--";
        case Kind::Less:         return "<";
        case Kind::LessEq:       return "<=";
        case Kind::Greater:      return ">";
        case Kind::GreaterEq:    return ">=";
        case Kind::Eq:           return "=";
        case Kind::EqEq:         return "==";
        case Kind::NotEq:        return "!=";
        case Kind::Shl:          return "<<";
        case Kind::ShlEq:        return "<<=";
        case Kind::Shr:          return ">>";
        case Kind::ShrEq:        return ">>=";
        case Kind::ModEq:        return "%=";
        case Kind::OrEq:         return "|=";
        case Kind::AndEq:        return "&=";
        case Kind::PlusEq:       return "+=";
        case Kind::MinusEq:      return "-=";
        case Kind::MulEq:        return "*=";
        case Kind::DivEq:        return "/=";
        case Kind::Qst:          return "?";
        case Kind::RightArrow:   return "->";
    
        // literals
        case Kind::IntLit:   return "IntLit";
        case Kind::FloatLit: return "FloatLit";
        case Kind::CharLit:  return "CharLit";
        case Kind::StrLit:   return "StrLit";
        case Kind::ArrLit:   return "ArrLit";
    
        // end of file
        case Kind::EndOfFile: return "EndOfFile";
    
        // invalid tokens
        case Kind::Invalid: return "Mongolien";
      }
    }
    // clang-format on

    static int precedence(const Kind kind) {
      switch (kind) {
        case Token::Kind::Eq:
          return 5;
        case Token::Kind::Plus:
        case Token::Kind::Minus:
          return 10;
        case Token::Kind::Mul:
        case Token::Kind::Div:
          return 20;
        default:
          return 0;
      }
    }
    static bool right_associative(const Kind kind) {
      return (kind == Token::Kind::Eq);
    }
  };
} // namespace phantom
