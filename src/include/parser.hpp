#ifndef PARSER_HPP
#define PARSER_HPP

#include "statement.hpp"
#include "token.hpp"
#include <vector>

class Parser {
    public:
        Parser(const std::vector<Token_T>& tokens);
        const std::vector<Statement_T>& parse();

    private:
        const std::vector<Token_T>& tokens;
        bool parsed;
        size_t idx;
        std::vector<Statement_T> statements;

        const TOKEN_TYPE get_token_type(size_t idx) const;
        Statement_T parse_statement();
        Argument_T parse_argument();
        bool is_end(size_t idx);
        bool is_line_terminator(size_t idx);
};

#endif
