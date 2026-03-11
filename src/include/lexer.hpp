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
        bool expecting_number;

        Token_T parse_identifier();
        Token_T parse_number();
        Token_T parse_symbol();

        void advance(size_t n);
};

#endif
