#include "include/lexer.hpp"
#include "include/lexer_specification.hpp"
#include "include/createBuffer.hpp"
#include "include/token.hpp"
#include <iostream>
#include <stdexcept>

Lexer::Lexer(const std::string& path) :
    tokenized(false),
    tokens(),
    buffer(create_file_buffer(path)),
    idx(0)
{}

const std::vector<Token_T>& Lexer::tokenize() {
    if (this->tokenized)
        return this->tokens;

    while (idx < buffer.size()) {
        idx = buffer.find_first_not_of(WHITESPACE, idx);
        if (idx == buffer.npos)
            return this->tokens;

        char c = buffer[idx];
        if (IDENTIFIER_START.find_first_of(c) != IDENTIFIER_START.npos) {
            this->tokens.push_back(this->parse_identifier());
        } else if (NUMBERS.find_first_of(c) != NUMBERS.npos) {
            this->tokens.push_back(this->parse_number());
        } else if (SYMBOLS.find_first_of(c) != SYMBOLS.npos) {
            this->tokens.push_back(this->parse_symbol());
        } else {
            throw std::runtime_error("Unexpected char: " + std::string(1, c));
        }
    }

    this->tokenized = true;
    return this->tokens;
}

Token_T Lexer::parse_identifier() {
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
    const size_t start = this->idx;
    size_t end = this->buffer.find_first_not_of(NUMBERS_HEX, start);
    if (end == this->buffer.npos) {
        end = this->buffer.size();
    }

    Token_T token;
    token.type = TOKEN_TYPE::NUMBER;
    token.value = std::string_view(this->buffer.data() + start, end - start);

    this->advance(end - start);
    return token;
}

Token_T Lexer::parse_symbol() {
    const size_t start = this->idx;
    size_t end = start + 1;
    const char symbol = this->buffer[start];

    Token_T token;
    TOKEN_TYPE type = symbol_to_type(symbol);
    switch (type) {
        case TOKEN_TYPE::DOLLAR:
            token.type = TOKEN_TYPE::NUMBER_HEX;
            end = this->buffer.find_first_not_of(NUMBERS_HEX, start+1);
            break;

        case TOKEN_TYPE::SEMI:
            token.type = TOKEN_TYPE::COMMENT;
            end = this->buffer.find_first_of('\n', start);
            break;

        case TOKEN_TYPE::INVALID:
            throw std::runtime_error("Unexpected symbol: " + std::string(1, symbol));
            break;

        default:
            token.type = type;
            break;
    }

    if (end == this->buffer.npos) {
        end = this->buffer.size();
    }
    const size_t len = end - start;
    token.value = std::string_view(this->buffer.data() + start, len);

    this->advance(len);
    return token;
}

void Lexer::advance(size_t n) {
    this->idx += n;
}
