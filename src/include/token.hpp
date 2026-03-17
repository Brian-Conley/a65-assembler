#ifndef TOKEN_HPP
#define TOKEN_HPP

#include <string_view>
#include <string>
#include <map>

enum class TOKEN_TYPE {
    IDENTIFIER,
    NUMBER_DEC,
    NUMBER_HEX,
    NUMBER_BIN,

    NEWLINE,    // \n

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

    UNKNOWN     // unrecognized token
};

extern const std::map<TOKEN_TYPE, std::string> type_to_str_lookup;

struct Token_T {
    TOKEN_TYPE type;
    std::string_view value;
};

TOKEN_TYPE symbol_to_type(char c);

const std::string_view type_to_str(TOKEN_TYPE t);

#endif
