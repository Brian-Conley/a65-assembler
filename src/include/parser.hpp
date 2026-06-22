#ifndef PARSER_HPP
#define PARSER_HPP

#include "statement.hpp"
#include "token.hpp"
#include <vector>

class Parser {
    public:
        /**
         * @brief The intended constructor for the Parser.
         *
         * @param tokens The vector of tokens produced by the Lexer.
         */
        Parser(const std::vector<Token_T>& tokens);

        /**
         * @brief Parse the token vector into statements.
         *
         * @return A const reference vector of statements.
         */
        const std::vector<Statement_T>& parse();

    private:
        /** @name Data Buffers */
        ///@{
        const std::vector<Token_T>& tokens;  /**< Token vector passed to the constructor. */
        std::vector<Statement_T> statements; /**< Container for all the statements. */
        ///@}

        /** @name State Management */
        ///@{
        bool parsed; /**< Flag to prevent duplicate work from multiple parse() calls. */
        size_t line; /**< Line number of the source */
        size_t idx;  /**< Index pointer for the token vector. */
        ///@}

        /*
         * @brief Gets the token type field at the specified index.
         *
         * @return Value of the token's type field.
         */
        TOKEN_TYPE get_token_type(size_t idx) const;

        /*
         * @brief Form a statement with tokens starting at the current idx.
         *
         * Takes the token at idx, moving forward parsing the components of a statement.
         * A statement is terminated by a line terminating token, or an end of buffer token.
         */
        Statement_T parse_statement();

        /*
         * @brief Form an argument using token indices ranging from idx to the first line terminating / end of buffer token.
         *
         * The range of the argument is in the form [start, end).
         */
        Argument_T parse_argument();

        /** @brief Check if the token at the specified index is the end of the buffer. */
        bool is_end(size_t idx);

        /** @brief Check if the token at the specified index is a line terminator. */
        bool is_line_terminator(size_t idx);
};

#endif
