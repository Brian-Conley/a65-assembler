#ifndef LEXER_HPP
#define LEXER_HPP

#include "token.hpp"
#include <vector>
#include <string>

class Lexer {
    public:
        Lexer(const std::string& path);
        const std::vector<Token_T>& tokenize();
    private:
        bool tokenized;
        std::vector<Token_T> tokens;
        const std::string buffer;
        size_t idx;

        Token_T parse_identifier();
        Token_T parse_number();
        Token_T parse_symbol();
        Token_T parse_newline();

        void advance(size_t n);
};

#endif
