#include "include/type_printers.hpp"
#include "include/statement.hpp"
#include <sstream>
#include <iomanip>

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

// This printer is vibe coded
const std::string statement_to_str(const std::vector<Token_T>& tokens, const Statement_T& s) {
    // ==========================================
    // STEP 1: Convert Statement Type Enum to String
    // ==========================================
    std::string type_str;
    switch (s.type) {
        case STATEMENT_TYPE::INSTRUCTION: type_str = "INSTRUCTION"; break;
        case STATEMENT_TYPE::DIRECTIVE:   type_str = "DIRECTIVE";   break;
        case STATEMENT_TYPE::LABEL_ONLY:  type_str = "LABEL_ONLY";  break;
    }

    // ==========================================
    // STEP 2: Extract Label Component if Present
    // ==========================================
    std::string label_str = "";
    if (s.label.has_value()) {
        label_str = std::string(tokens[s.label.value()].value);
    }

    // ==========================================
    // STEP 3: Extract Mnemonic Component if Present
    // ==========================================
    std::string mnemonic_str = "";
    if (s.mnemonic.has_value()) {
        mnemonic_str = std::string(tokens[s.mnemonic.value()].value);
    }

    // ==========================================
    // STEP 4: Reconstruct Argument String Range
    // ==========================================
    std::string arg_str = "";
    if (s.argument.has_value()) {
        // Step through the atomic argument tokens and append them sequentially
        for (size_t i = s.argument.value().start; i < s.argument.value().end; ++i) {
            arg_str.append(std::string(tokens[i].value) + ' ');
        }
    }

    // ==========================================
    // STEP 5: Pipe Components Into Formatted Stream
    // ==========================================
    std::ostringstream ss;

    // Column 1: Line number (Right-aligned, e.g., "[  14]: ")
    ss << "[" << std::setw(4) << std::right << s.line << "]: ";

    // Column 2: Statement Type (Left-aligned, bracketed, e.g., "<INSTRUCTION> ")
    ss << std::setw(15) << std::left << ("<" + type_str + ">") << " | ";

    // Column 3: Label Component (Left-aligned, bracketed, e.g., "[MAIN_LOOP]: ")
    // We adjust the width padding based on whether a label actually exists
    if (!label_str.empty()) {
        ss << std::setw(15) << std::left << ("[" + label_str + "]:");
    } else {
        // Keep the column layout perfectly aligned even if the label is blank
        ss << std::setw(15) << std::left << "";
    }

    // Column 4 & 5: Mnemonic and Argument
    // Pad the mnemonic column so arguments line up vertically
    ss << " " << std::setw(5) << std::left << mnemonic_str;
    ss << " " << arg_str;

    return ss.str();
}
