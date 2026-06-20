#include "include/lexer.hpp"
#include "include/lexer_specification.hpp"
#include "include/createBuffer.hpp"
#include "include/status_code.hpp"
#include "include/token.hpp"
#include <iostream>

Lexer::Lexer(const std::string& path) :
    tokenized(false),
    tokens(),
    buffer(create_file_buffer(path)),
    idx(0)
{}

const std::vector<Token_T>& Lexer::tokenize() {
    if (this->tokenized)
        return this->tokens;

    // Pre allocate some space, assuming one token roughly every 3 chars.
    // This decision is based on the fact that 1-3 char tokens
    //  are extremely common, and reallocating memory for vector
    //  growth is expensive.
    if (this->buffer.size() != 0) {
        this->tokens.reserve(this->buffer.size() / 3);
    }

    while (idx < buffer.size()) {
        // Skip all whitespace, return if end of buffer
        idx = buffer.find_first_not_of(WHITESPACE, idx);
        if (idx == buffer.npos)
            return this->tokens;

        // Create token based on what the current char is
        char c = buffer[idx];
        if (IDENTIFIER_START.find_first_of(c) != IDENTIFIER_START.npos) {
            this->tokens.push_back(this->parse_identifier());
        } else if (NUMBERS_DEC.find_first_of(c) != NUMBERS_DEC.npos) {
            this->tokens.push_back(this->parse_number());
        } else if (SYMBOLS.find_first_of(c) != SYMBOLS.npos) {
            this->tokens.push_back(this->parse_symbol());
        } else if (NEWLINE.find_first_of(c) != NEWLINE.npos) {
            this->tokens.push_back(this->parse_newline());
        } else {
            // TODO Improve error handling
            std::cerr << "Unknown char: " << c << '\n';
            exit(static_cast<int>(STATUS_CODE::LEXER_ERROR));
        }
    }

    // Append an end token to the end
    Token_T end_token;
    end_token.type = TOKEN_TYPE::END;
    this->tokens.push_back(end_token);

    this->tokenized = true;
    return this->tokens;
}

Token_T Lexer::parse_identifier() {
    // Set current char as the start, find the first non-identifier char
    // Create an identifier token of the content spanning start-end
    const size_t start = this->idx;

    size_t end = this->buffer.find_first_not_of(IDENTIFIER_CONTENT, start);
    if (end == this->buffer.npos) {
        end = this->buffer.size();
    }

    Token_T token;
    token.type = TOKEN_TYPE::IDENTIFIER;
    token.value = std::string_view(this->buffer.data() + start, end - start);

    this->advance(end - start);
    return token;
}

Token_T Lexer::parse_number() {
    // Set current char as the start, find the first non-decimal number char
    // Create a decimal number token of the content spanning start-end
    const size_t start = this->idx;
    size_t end = this->buffer.find_first_not_of(NUMBERS_DEC, start);
    if (end == this->buffer.npos) {
        end = this->buffer.size();
    }

    Token_T token;
    token.type = TOKEN_TYPE::NUMBER_DEC;
    token.value = std::string_view(this->buffer.data() + start, end - start);

    this->advance(end - start);
    return token;
}

Token_T Lexer::parse_symbol() {
    // Set current char as the start
    const size_t start = this->idx;
    size_t end = start + 1;
    const char symbol = this->buffer[start];

    // Assume the token type based on the symbol, then check edge cases.
    Token_T token;
    TOKEN_TYPE type = symbol_to_type(symbol);
    switch (type) {
        case TOKEN_TYPE::DOLLAR:
            // Find the first non-hex number char
            // Create a hex number token of the content spanning start-end
            token.type = TOKEN_TYPE::NUMBER_HEX;
            end = this->buffer.find_first_not_of(NUMBERS_HEX, start+1);
            break;

        case TOKEN_TYPE::PERCENT:
            // Find the first non-binary number char
            // Create a binary number token of the content spanning start-end
            token.type = TOKEN_TYPE::NUMBER_BIN;
            end = this->buffer.find_first_not_of(NUMBERS_BIN, start+1);
            break;

        case TOKEN_TYPE::SEMI:
            // Find the end of the line
            // Create a comment token of the line's remaining content
            token.type = TOKEN_TYPE::COMMENT;
            end = this->buffer.find_first_of('\n', start);
            break;

        // TODO Improve error handling
        /*
        case TOKEN_TYPE::UNKNOWN:
            throw std::runtime_error("Unexpected symbol: " + std::string(1, symbol));
            break;
        */

        default:
            token.type = type;
            break;
    }

    // Create the token and return
    if (end == this->buffer.npos) {
        end = this->buffer.size();
    }
    const size_t len = end - start;
    token.value = std::string_view(this->buffer.data() + start, len);

    this->advance(len);
    return token;
}

Token_T Lexer::parse_newline() {
    // Create a newline token and push the index forward.
    Token_T token;
    token.type = TOKEN_TYPE::NEWLINE;

    const size_t start = this->idx;
    const size_t end = start + 1;
    const size_t len = end - start;
    token.value = std::string_view(this->buffer.data() + start, len);

    this->advance(len);
    return token;
}

void Lexer::advance(size_t n) {
    this->idx += n;
}
