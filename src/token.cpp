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
            return TOKEN_TYPE::INVALID;
    }
}
