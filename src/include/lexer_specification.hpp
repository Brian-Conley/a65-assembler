#ifndef LEXER_SPECIFICATION_HPP
#define LEXER_SPECIFICATION_HPP

#include <string_view>

constexpr std::string_view WHITESPACE = " \t\n\r";

constexpr std::string_view IDENTIFIER_START =
    "abcdefghijklmnopqrstuvwxyz"
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "_";

constexpr std::string_view IDENTIFIER_CONTENT =
    "abcdefghijklmnopqrstuvwxyz"
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "0123456789"
    "_";

constexpr std::string_view NUMBERS_DEC =
    "1234567890";

constexpr std::string_view NUMBERS_HEX =
    "1234567890"
    "abcdefABCDEF";

constexpr std::string_view NUMBERS_BIN =
    "01";

constexpr std::string_view SYMBOLS =
    "#$%()+-*/,.:;";

#endif
