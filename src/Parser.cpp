#include <Parser.hpp>

namespace phantom {

    Token Parser::consume() {}
    Token Parser::peek(off_t offset) const {}
    bool Parser::match(Token::Kind token, off_t offset) const {}
}
