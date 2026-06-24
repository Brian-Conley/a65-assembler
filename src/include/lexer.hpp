#ifndef LEXER_HPP
#define LEXER_HPP

#include "token.hpp"
#include <vector>
#include <string>

class Lexer {
    public:
        /**
         * @brief The intended constructor for the Lexer.
         *
         * @param path The path of the target file.
         */
        Lexer(const std::string& path);

        /**
         * @brief Tokenize the contents of the file provided in the constructor.
         *
         * @return A const reference vector of the tokenized file.
         */
        const std::vector<Token_T>& tokenize();
    private:
        const std::string filename; /**< Name/path of the provided file. */

        /** @name Data Buffers */
        ///@{
        const std::string buffer;    /**< String buffer of the file. */
        std::vector<Token_T> tokens; /**< Container for all tokens. */
        ///@}

        /** @name State Management */
        ///@{
        bool tokenized; /**< Flag to prevent duplicate work from multiple tokenize() calls. */
        size_t idx;     /**< Index pointer for the file buffer. */
        size_t line;    /**< Track track the current line of the file. */
        ///@}

        /**
         * @brief Creates a token of an identifier.
         *
         * @return An identifier token.
         */
        Token_T parse_identifier();

        /**
         * @brief Creates a token of a decimal number.
         *
         * @return A decimal number token.
         */
        Token_T parse_number();

        /**
         * @brief Creates a token relevant to the symbol at idx.
         *
         * @return A token associated with the given symbol.
         */
        Token_T parse_symbol();

        /**
         * @brief Creates a newline token.
         *
         * @return A newline token.
         */
        Token_T parse_newline();

        /**
         * @brief Increments idx.
         * @note This does not do bounds checking.
         */
        void advance(size_t n);
};

#endif
