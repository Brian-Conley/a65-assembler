#include "include/parser.hpp"
#include "include/token.hpp"
#include "include/type_printers.hpp"
#include "include/status_code.hpp"
#include <cstdlib>
#include <iostream>

Parser::Parser(const std::vector<Token_T>& tokens):
    tokens(tokens),
    statements(),
    parsed(false),
    line(1),
    idx(0)
{}

const std::vector<Statement_T>& Parser::parse() {
    if (this->parsed) {
        return this->statements;
    }

    // Pre allocate some space, assuming one statement roughly every 4 tokens.
    // This is based on a personal test on a one-million line source file
    //      where there was one statement per ~4 tokens.
    if (this->tokens.size() != 0) {
        this->statements.reserve(this->tokens.size() / 4);
    }

    while (!this->parsed) {
        const TOKEN_TYPE token_type = this->get_token_type(this->idx);

        switch (token_type) {
            case TOKEN_TYPE::IDENTIFIER:
            case TOKEN_TYPE::DOT:
                this->statements.push_back(this->parse_statement());
                break;

            // Skip the token
            case TOKEN_TYPE::NEWLINE:
                ++this->line;
                ++this->idx;
                continue;
            case TOKEN_TYPE::COMMENT:
                ++this->idx;
                continue;
                break;

            // Exit if the token is unknown
            // TODO Improve error handling
            case TOKEN_TYPE::UNKNOWN:
                std::cerr << "Unknown token:\n"
                    << "\t   idx: " << this->idx << '\n'
                    << "\t token: " << token_to_str(this->tokens[this->idx]) << '\n';
                exit(static_cast<int>(STATUS_CODE::PARSER_ERROR));
                break;

            case TOKEN_TYPE::END:
                this->parsed = true;
                break;
            default:
                // TODO Improve error handling
                std::cerr << "Unexpected token:\n"
                    << "\t  idx: " << this->idx << '\n'
                    << "\ttoken: " << token_to_str(this->tokens[this->idx]) << '\n';
                exit(static_cast<int>(STATUS_CODE::PARSER_ERROR));
        }
    }

    return this->statements;
}

TOKEN_TYPE Parser::get_token_type(size_t idx) const {
    // TODO Improve error handling
    if (idx >= this->tokens.size()) {
        std::cerr << "Parser::get_token_type() out of bounds\n"
            << "\t              idx: " << idx << '\n'
            << "\ttoken vector size: " << this->tokens.size() << '\n';

        exit(static_cast<int>(STATUS_CODE::PARSER_ERROR));
    }
    return this->tokens[idx].type;
}

Statement_T Parser::parse_statement() {
    /*
     * Parse for:
     * 1. Possible label (is next token COLON?)
     * 2. If yes, set the label, inc the index, look for
     *    directive or instruction after (mnemonic)
     * 3. Potential argument following the mnemonic
     */
    Statement_T statement;
    statement.line = this->line;

    TOKEN_TYPE token_type = this->get_token_type(this->idx);

    // Get a possible label
    if (token_type == TOKEN_TYPE::IDENTIFIER) {
        if (this->get_token_type(this->idx + 1) == TOKEN_TYPE::COLON) {
            statement.label = this->idx;
            this->idx += 2;

            // Check for EOL
            if (this->is_line_terminator(this->idx)) {
                statement.type = STATEMENT_TYPE::LABEL_ONLY;
                // this->idx += 1;
                return statement;
            }

            // Check for END
            // Do not increment further if it's the final token.
            // The main parser loop will handle this case.
            if (this->is_end(this->idx)) {
                statement.type = STATEMENT_TYPE::LABEL_ONLY;
                return statement;
            }

        }
    }

    // Update token type to the possible new index
    token_type = this->get_token_type(this->idx);

    switch (token_type) {
        case TOKEN_TYPE::IDENTIFIER:
            statement.type = STATEMENT_TYPE::INSTRUCTION;
            statement.mnemonic = this->idx;
            ++this->idx;

            // Check for EOL
            if (this->is_line_terminator(this->idx)) {
                // Skip token and return
                ++this->idx;
                return statement;
            }

            // Check for END
            // Return WITHOUT skipping
            // The main parser loop will handle this case
            if (this->is_end(this->idx)) {
                return statement;
            }

            statement.argument = this->parse_argument();

            break;

        case TOKEN_TYPE::DOT:
            statement.type = STATEMENT_TYPE::DIRECTIVE;
            ++this->idx;

            // TODO Improve error handling
            if (get_token_type(this->idx) != TOKEN_TYPE::IDENTIFIER) {
                std::cerr << "Unexpected token:\n"
                    << "\t  idx: " << this->idx << '\n'
                    << "\ttoken: " << token_to_str(this->tokens[this->idx]);
                exit(static_cast<int>(STATUS_CODE::PARSER_ERROR));
            }

            statement.mnemonic = this->idx;
            ++this->idx;

            // Check for EOL
            if (this->is_line_terminator(this->idx)) {
                // Skip token and return
                ++this->idx;
                return statement;
            }

            // Check for END
            // Return WITHOUT skipping
            // The main parser loop will handle this case
            if (this->is_end(this->idx)) {
                return statement;
            }

            statement.argument = this->parse_argument();

            break;

        default:
            // TODO Improve error handling
            std::cerr << "Unexpected token:\n"
                << "\t  idx: " << this->idx << '\n'
                << "\ttoken: " << token_to_str(this->tokens[this->idx]);
            exit(static_cast<int>(STATUS_CODE::PARSER_ERROR));
            break;
    }

    return statement;
}

Argument_T Parser::parse_argument() {
    Argument_T arg;
    arg.start = this->idx;
    while (true) {
        ++this->idx;
        if (is_line_terminator(this->idx) || is_end(this->idx)) {
            break;
        }

        // TODO Improve error handling
        if (this->idx >= this->tokens.size()) {
            std::cerr << "Parser::parse_argument():\n"
                << "\tidx exceeded the bounds of the token vector.\n";
            exit(static_cast<int>(STATUS_CODE::PARSER_ERROR));
        }
    }
    arg.end = this->idx;
    return arg;
}

bool Parser::is_end(size_t idx) {
    return this->tokens[idx].type == TOKEN_TYPE::END;
}

bool Parser::is_line_terminator(size_t idx) {
    TOKEN_TYPE token_type = this->tokens[idx].type;
    return token_type == TOKEN_TYPE::NEWLINE || token_type == TOKEN_TYPE::COMMENT;
}
