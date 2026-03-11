#ifndef TOKEN_HPP
#define TOKEN_HPP

#include <string_view>

enum class TOKEN_TYPE {
    IDENTIFIER,
    NUMBER_DEC,
    NUMBER_HEX,
    NUMBER_BIN,

    HASH,       // #
    DOLLAR,     // $
    PERCENT,    // %

    LPAREN,     // (
    RPAREN,     // )

    PLUS,       // +
    MINUS,      // -
    STAR,       // *
    SLASH,      // /
    COMMA,      // ,

    DOT,        // .
    COLON,      // :
    SEMI,       // ;
    COMMENT,    // derived from SEMI by the Lexer

    INVALID     // used to propagate errors
};

struct Token_T {
    TOKEN_TYPE type;
    std::string_view value;
};

TOKEN_TYPE symbol_to_type(char c);

#endif
