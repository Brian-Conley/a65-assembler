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

    LANGLE,     // <
    RANGLE,     // >

    PLUS,       // +
    MINUS,      // -
    STAR,       // *
    SLASH,      // /
    COMMA,      // ,

    DOT,        // .
    COLON,      // :
    SEMI,       // ;
    COMMENT,    // derived from SEMI by the Lexer
    QUOTE,      // "
    STRING_LITERAL, // derived from QUOTE by the Lexer

    END,        // appended to the end of the token vector
    UNKNOWN     // unrecognized token
};

extern const std::map<TOKEN_TYPE, std::string> token_type_to_str_lookup;

/** * Convert the token type into a string matching the type name. */
const std::string_view token_type_to_str(TOKEN_TYPE t);

/** * Container for token information. */
struct Token_T {
    TOKEN_TYPE type;        /**< The category of tokens. */
    std::string_view value; /**< The actual buffer text associated with the token. */
};

/** @brief Determine a token type based on the given character. */
TOKEN_TYPE symbol_to_type(char c);

#endif
