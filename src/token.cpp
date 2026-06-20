#include "include/token.hpp"

TOKEN_TYPE symbol_to_type(char c) {
    switch (c) {
        case '#':
            return TOKEN_TYPE::HASH;
        case '$':
            return TOKEN_TYPE::DOLLAR;
        case '%':
            return TOKEN_TYPE::PERCENT;
        case '(':
            return TOKEN_TYPE::LPAREN;
        case ')':
            return TOKEN_TYPE::RPAREN;
        case '<':
            return TOKEN_TYPE::LANGLE;
        case '>':
            return TOKEN_TYPE::RANGLE;
        case '+':
            return TOKEN_TYPE::PLUS;
        case '-':
            return TOKEN_TYPE::MINUS;
        case '*':
            return TOKEN_TYPE::STAR;
        case '/':
            return TOKEN_TYPE::SLASH;
        case ',':
            return TOKEN_TYPE::COMMA;
        case '.':
            return TOKEN_TYPE::DOT;
        case ':':
            return TOKEN_TYPE::COLON;
        case ';':
            return TOKEN_TYPE::SEMI;
        default:
            return TOKEN_TYPE::UNKNOWN;
    }
}

const std::map<TOKEN_TYPE, std::string> token_type_to_str_lookup = {
    {TOKEN_TYPE::IDENTIFIER, "IDENTIFIER"},
    {TOKEN_TYPE::NUMBER_DEC, "NUMBER_DEC"},
    {TOKEN_TYPE::NUMBER_HEX, "NUMBER_HEX"},
    {TOKEN_TYPE::NUMBER_BIN, "NUMBER_BIN"},

    {TOKEN_TYPE::NEWLINE, "NEWLINE"},

    {TOKEN_TYPE::HASH, "HASH"},
    {TOKEN_TYPE::DOLLAR, "DOLLAR"},
    {TOKEN_TYPE::PERCENT, "PERCENT"},

    {TOKEN_TYPE::LPAREN, "LPAREN"},
    {TOKEN_TYPE::RPAREN, "RPAREN"},

    {TOKEN_TYPE::LANGLE, "LANGLE"},
    {TOKEN_TYPE::RANGLE, "RANGLE"},

    {TOKEN_TYPE::PLUS, "PLUS"},
    {TOKEN_TYPE::MINUS, "MINUS"},
    {TOKEN_TYPE::STAR, "STAR"},
    {TOKEN_TYPE::SLASH, "SLASH"},
    {TOKEN_TYPE::COMMA, "COMMA"},

    {TOKEN_TYPE::DOT, "DOT"},
    {TOKEN_TYPE::COLON, "COLON"},
    {TOKEN_TYPE::SEMI, "SEMI"},
    {TOKEN_TYPE::COMMENT, "COMMENT"},

    {TOKEN_TYPE::END, "END"},
    {TOKEN_TYPE::UNKNOWN, "UNKNOWN"}
};

const std::string_view token_type_to_str(TOKEN_TYPE t) {
    return token_type_to_str_lookup.at(t);
}
