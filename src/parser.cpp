#include "include/parser.hpp"
#include "include/token.hpp"
#include "include/type_printers.hpp"
#include "include/status_code.hpp"
#include "include/diagnostics.hpp"
#include <cstdlib>
#include <iostream>

Parser::Parser(const std::vector<Token_T>& tokens, const std::string& filename):
    filename(filename),
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

            case TOKEN_TYPE::UNKNOWN:
                Diagnostics::get().report({
                        DiagnosticSeverity::Error,
                        DiagnosticCode::ParserUnknownToken,
                        {filename, line},
                        (std::string("Unknown token: \"") + token_to_str(tokens[idx]) + "\"")
                        });
                break;

            case TOKEN_TYPE::END:
                this->parsed = true;
                break;
            default:
                Diagnostics::get().report({
                        DiagnosticSeverity::Error,
                        DiagnosticCode::ParserUnexpectedToken,
                        {filename, line},
                        (std::string("Unexpected token: \"") + token_to_str(tokens[idx]) + "\"")
                        });
                break;
        }
    }

    return this->statements;
}

TOKEN_TYPE Parser::get_token_type(size_t idx) const {
    if (idx >= this->tokens.size()) {
        Diagnostics::get().report({
                DiagnosticSeverity::Fatal,
                DiagnosticCode::ParserOutOfBounds,
                {filename, line},
                "Parser::get_token_type() out of bounds."
                });
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

            if (get_token_type(this->idx) != TOKEN_TYPE::IDENTIFIER) {
                Diagnostics::get().report({
                        DiagnosticSeverity::Error,
                        DiagnosticCode::ParserUnexpectedToken,
                        {filename, line},
                        (std::string("Unexpected token: \"") + token_to_str(tokens[idx]) + "\"")
                        });
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
            Diagnostics::get().report({
                    DiagnosticSeverity::Error,
                    DiagnosticCode::ParserUnexpectedToken,
                    {filename, line},
                    (std::string("Unexpected token: \"") + token_to_str(tokens[idx]) + "\"")
                    });
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

        if (this->idx >= this->tokens.size()) {
            Diagnostics::get().report({
                    DiagnosticSeverity::Fatal,
                    DiagnosticCode::ParserOutOfBounds,
                    {filename, line},
                    "Parser::parse_argument() out of bounds."
                    });
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
