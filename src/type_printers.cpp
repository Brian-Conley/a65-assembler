#include "include/type_printers.hpp"
#include "include/statement.hpp"

const std::string token_to_str(const Token_T& t) {
    if (t.type == TOKEN_TYPE::NEWLINE) {
        return "<NEWLINE>\n";
    }

    if (t.type == TOKEN_TYPE::END) {
        return "<END>";
    }

    return "<" + std::string(token_type_to_str(t.type)) + ": "
        + std::string(t.value) + ">";
}

const std::string statement_to_str(const std::vector<Token_T>& tokens, const Statement_T& s) {
    std::string type;
    switch (s.type) {
        case STATEMENT_TYPE::INSTRUCTION:
            type = "INSTRUCTION";
            break;
        case STATEMENT_TYPE::DIRECTIVE:
            type = "DIRECTIVE";
            break;
        case STATEMENT_TYPE::LABEL_ONLY:
            type = "LABEL_ONLY";
            break;
    }
    std::string label = s.label.has_value() ? std::string(tokens[s.label.value()].value) : "no_label";
    std::string mnemonic = s.mnemonic.has_value() ? std::string(tokens[s.mnemonic.value()].value) : "no_mnemonic";
    std::string arg = "";
    if (s.argument.has_value()) {
        for (size_t i = s.argument.value().start; i < s.argument.value().end; ++i) {
            arg.append(std::string(tokens[i].value));
        }
    } else {
        arg = "no_args";
    }

    return "<"+type+"> | "+"["+label+"]: "+mnemonic+", "+arg;
}
