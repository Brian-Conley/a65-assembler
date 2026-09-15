#include "include/assembler.hpp"
#include "include/opcodes.hpp"
#include "include/statement.hpp"
#include "include/token.hpp"
#include "include/hash.hpp"
#include "include/diagnostics.hpp"
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

Assembler::Assembler(const std::vector<Token_T>& tokens,
        const std::vector<Statement_T>& statements,
        const std::string& filename) :
    filename(filename),
    tokens(tokens),
    statements(statements),
    assembled(false),
    is_pass_1(true),
    reset_vector(0x0000),
    reset_vector_initialized(false)
{}

const std::vector<uint8_t>& Assembler::assemble() {

    if (assembled)
        return machine_code;

    // Pass 1: Build the initial symbol table, assume all forward references are 2-bytes.
    uint16_t pc = reset_vector;
    for (const auto& s : statements) {

        if (s.label.has_value()) {
            handle_label(s, pc, true);
        }

        if (s.type == STATEMENT_TYPE::LABEL_ONLY) {
            continue;
        }

        if (s.type == STATEMENT_TYPE::DIRECTIVE) {
            handle_directive(s, pc);
            continue;
        }

        /* INSTRUCTION TYPE */

        if (!s.mnemonic.has_value()) {
            Diagnostics::get().report({
                    DiagnosticSeverity::Error,
                    DiagnosticCode::AssemblerNoMnemonic,
                    {filename, s.line},
                    "No instruction mnemonic found."
                    });
        }

        std::optional<INSTRUCTION_TYPE> ins_type = determine_instruction_type(tokens[s.mnemonic.value()]);
        if (!ins_type.has_value()) {
            Diagnostics::get().report({
                    DiagnosticSeverity::Error,
                    DiagnosticCode::AssemblerUnknownInstruction,
                    {filename, s.line},
                    (std::string("Unknown instruction mnemonic: \"" + std::string(token_val(s.mnemonic.value())) + "\"."))
                    });
            continue;
        }

        auto addr_mode = determine_addressing_mode(ins_type.value(), s);
        if (!addr_mode.has_value()) {
            Diagnostics::get().report({
                    DiagnosticSeverity::Error,
                    DiagnosticCode::AssemblerUnresolvedAddressMode,
                    {filename, s.line},
                    "Failed to resolve address mode."
                    });
            continue;
        }
        pc += get_mode_size(addr_mode.value());

    }
    is_pass_1 = false;
    machine_code.clear();
    reset_vector_initialized = false;

    // Pass 2 through n-1: Iterate until the symbol table stabilizes.
    bool label_changed = true;
    size_t iterations = 0;
    while (label_changed && iterations < 5) {

        pc = reset_vector;
        label_changed = false;
        for (const auto& s : statements) {

            if (s.label.has_value()) {
                if (handle_label(s, pc)) {
                    label_changed = true;
                }
            }

            if (s.type == STATEMENT_TYPE::LABEL_ONLY) {
                continue;
            }

            if (s.type == STATEMENT_TYPE::DIRECTIVE) {
                handle_directive(s, pc);
                continue;
            }

            /* INSTRUCTION TYPE */

            std::optional<INSTRUCTION_TYPE> ins_type = determine_instruction_type(tokens[s.mnemonic.value()]);
            if (!ins_type.has_value()) {
                // Error already reported in pass 1
                continue;
            }

            auto addr_mode = determine_addressing_mode(ins_type.value(), s);
            if (!addr_mode.has_value()) {
                // Error already reported in pass 1
                continue;
            }
            pc += get_mode_size(addr_mode.value());

        }

        machine_code.clear();
        reset_vector_initialized = false;
        ++iterations;
    }
    machine_code.clear();
    reset_vector_initialized = false;

    // Pass n: Generate the machine code
    pc = reset_vector;
    for (auto s : statements) {

        if (s.type == STATEMENT_TYPE::LABEL_ONLY)
            continue;

        if (s.type == STATEMENT_TYPE::DIRECTIVE) {
            handle_directive(s, pc);
            continue;
        }

        /* INSTRUCTION */

        std::optional<INSTRUCTION_TYPE> ins_type_opt = determine_instruction_type(tokens[s.mnemonic.value()]);
        if (!ins_type_opt.has_value()) {
            // Error already reported in pass 1
            continue;
        }

        INSTRUCTION_TYPE ins_type = ins_type_opt.value();
        auto addr_mode_opt = determine_addressing_mode(ins_type, s);
        if (!addr_mode_opt.has_value()) {
            // Error already reported in pass 1
            continue;
        }
        auto addr_mode = addr_mode_opt.value();

        if (opcode_lookup[ins_type][addr_mode] == BAD_OPC) {
            Diagnostics::get().report({
                    DiagnosticSeverity::Error,
                    DiagnosticCode::AssemblerBadAddressMode,
                    {filename, s.line},
                    "Addressing mode not supported by instruction."
                    });
        }

        if (addr_mode == ADD_A || addr_mode == ADD_IMPLIED) {
            emit_byte(opcode_lookup[ins_type][addr_mode], pc);
            continue;
        }

        std::optional<uint16_t> arg_val_opt = resolve_ins_arg(s.argument.value(), addr_mode);
        if (!arg_val_opt.has_value()) {
            Diagnostics::get().report({
                    DiagnosticSeverity::Error,
                    DiagnosticCode::AssemblerUnresolvedArg,
                    {filename, s.line},
                    "Failed to resolve instruction argument."
                    });
        }
        uint16_t arg_val = arg_val_opt.value();
        size_t ins_size = get_mode_size(addr_mode);

        if (addr_mode == ADD_REL) {
            int difference = arg_val - (pc + 2);
            if (difference < -128 || difference > 127) {
                Diagnostics::get().report({
                        DiagnosticSeverity::Error,
                        DiagnosticCode::AssemblerJumpTooLarge,
                        {filename, s.line},
                        "Branch offset too great: " + std::to_string(difference) + " (must be within -128/+127)."
                        });
            }
            uint8_t jump = static_cast<uint8_t>(difference);

            emit_byte(opcode_lookup[ins_type][addr_mode], pc);
            emit_byte(jump, pc);
            continue;
        }

        emit_byte(opcode_lookup[ins_type][addr_mode], pc);
        emit_byte(arg_val & 0xFF, pc);
        if (ins_size == 3) {
            emit_byte((arg_val >> 8) & 0xFF, pc);
        }

    }

    return this->machine_code;
}

std::optional<INSTRUCTION_TYPE> Assembler::determine_instruction_type(const Token_T& t) const {

    switch (hash_djb2(t.value)) {
        case "ADC"_hash:
            return INS_ADC;
        case "AND"_hash:
            return INS_AND;
        case "ASL"_hash:
            return INS_ASL;
        case "BCC"_hash:
            return INS_BCC;
        case "BCS"_hash:
            return INS_BCS;
        case "BEQ"_hash:
            return INS_BEQ;
        case "BIT"_hash:
            return INS_BIT;
        case "BMI"_hash:
            return INS_BMI;
        case "BNE"_hash:
            return INS_BNE;
        case "BPL"_hash:
            return INS_BPL;
        case "BRK"_hash:
            return INS_BRK;
        case "BVC"_hash:
            return INS_BVC;
        case "BVS"_hash:
            return INS_BVS;
        case "CLC"_hash:
            return INS_CLC;
        case "CLD"_hash:
            return INS_CLD;
        case "CLI"_hash:
            return INS_CLI;
        case "CLV"_hash:
            return INS_CLV;
        case "CMP"_hash:
            return INS_CMP;
        case "CPX"_hash:
            return INS_CPX;
        case "CPY"_hash:
            return INS_CPY;
        case "DEC"_hash:
            return INS_DEC;
        case "DEX"_hash:
            return INS_DEX;
        case "DEY"_hash:
            return INS_DEY;
        case "EOR"_hash:
            return INS_EOR;
        case "INC"_hash:
            return INS_INC;
        case "INX"_hash:
            return INS_INX;
        case "INY"_hash:
            return INS_INY;
        case "JMP"_hash:
            return INS_JMP;
        case "JSR"_hash:
            return INS_JSR;
        case "LDA"_hash:
            return INS_LDA;
        case "LDX"_hash:
            return INS_LDX;
        case "LDY"_hash:
            return INS_LDY;
        case "LSR"_hash:
            return INS_LSR;
        case "NOP"_hash:
            return INS_NOP;
        case "ORA"_hash:
            return INS_ORA;
        case "PHA"_hash:
            return INS_PHA;
        case "PHP"_hash:
            return INS_PHP;
        case "PLA"_hash:
            return INS_PLA;
        case "PLP"_hash:
            return INS_PLP;
        case "ROL"_hash:
            return INS_ROL;
        case "ROR"_hash:
            return INS_ROR;
        case "RTI"_hash:
            return INS_RTI;
        case "RTS"_hash:
            return INS_RTS;
        case "SBC"_hash:
            return INS_SBC;
        case "SEC"_hash:
            return INS_SEC;
        case "SED"_hash:
            return INS_SED;
        case "SEI"_hash:
            return INS_SEI;
        case "STA"_hash:
            return INS_STA;
        case "STX"_hash:
            return INS_STX;
        case "STY"_hash:
            return INS_STY;
        case "TAX"_hash:
            return INS_TAX;
        case "TAY"_hash:
            return INS_TAY;
        case "TSX"_hash:
            return INS_TSX;
        case "TXA"_hash:
            return INS_TXA;
        case "TXS"_hash:
            return INS_TXS;
        case "TYA"_hash:
            return INS_TYA;
        default:
            return std::nullopt;
    };
}

std::optional<ADDRESSING_MODE> Assembler::determine_addressing_mode(INSTRUCTION_TYPE ins, const Statement_T& s) const {

    auto arg = s.argument;

    // Check for Implied
    if (!arg.has_value()) {
        return ADD_IMPLIED;
    }

    size_t start = arg->start;
    size_t end = arg->end - 1; // -1 because of the arg range [start, end)
    std::string_view first_token_val = token_val(start);
    std::string_view end_token_val = token_val(end);

    // Check if the operand will be one byte using a '<' or '>' operator
    bool prefer_zpg = false;

    // Check for Accumulator mode
    if (first_token_val == "A" || first_token_val == "a")
        return ADD_A;

    // Check for instructions that use relative mode
    if (opcode_lookup[ins][ADD_REL] != BAD_OPC)
        return ADD_REL;

    // Check for immediate mode
    if (first_token_val == "#")
        return ADD_IMM;

    // Check for indirect modes
    if (first_token_val == "(") {
        if (end_token_val == "Y" || end_token_val == "y")
            return ADD_INDY;
        std::string_view xcheck = token_val(end - 1);
        if (xcheck == "X" || xcheck == "x")
            return ADD_INDX;
        return ADD_IND;
    }

    // Check for X/Y indexing
    bool x_indexed = (end_token_val == "X" || end_token_val == "x");
    bool y_indexed = (end_token_val == "Y" || end_token_val == "y");

    // Find first token containing the operand
    // 0 should be safe to use because an operand shouldn't be able to exist as token 0
    size_t operand_idx = 0;
    for (size_t i = start; i <= end; ++i) {
        TOKEN_TYPE t = this->tokens[i].type;
        if (t == TOKEN_TYPE::NUMBER_BIN || t == TOKEN_TYPE::NUMBER_DEC
        ||  t == TOKEN_TYPE::NUMBER_HEX || t == TOKEN_TYPE::IDENTIFIER) {

            operand_idx = i;
            break;
        }

        if (i >= end) {
            // Reached end of argument without finding a suitable token
            return std::nullopt;
        }
    }

    std::optional<uint16_t> value;
    TOKEN_TYPE tok_type = this->tokens[operand_idx].type;
    switch(tok_type) {
        // Look for an associated symbol value.
        case TOKEN_TYPE::IDENTIFIER:
            // If the symbol isn't found, assume a 2-byte value
            value = resolve_symbol_address(tokens[operand_idx].value).value_or(0xFFFF);
            break;

        // Resolve numeric value if not a symbol.
        case TOKEN_TYPE::NUMBER_BIN:
        case TOKEN_TYPE::NUMBER_DEC:
        case TOKEN_TYPE::NUMBER_HEX: {
            value = resolve_numeric_token_value(tokens[operand_idx]);
            break;
        }

        default:
            return std::nullopt;

    };

    if (!value.has_value()) {
        return std::nullopt;
    }

    // Distinguish between ABS and ZPG and return.
    if (value <= 0xFF)
        prefer_zpg = true;

    if (x_indexed)
        return prefer_zpg ? ADD_ZPGX : ADD_ABSX;
    if (y_indexed)
        return prefer_zpg ? ADD_ZPGY : ADD_ABSY;
    return prefer_zpg ? ADD_ZPG : ADD_ABS;
}

size_t Assembler::get_mode_size(ADDRESSING_MODE mode) const {
    switch(mode) {
        case ADD_A:
        case ADD_IMPLIED:
            return 1;
        case ADD_IMM:
        case ADD_REL:
        case ADD_ZPG:
        case ADD_ZPGX:
        case ADD_ZPGY:
        case ADD_INDX:
        case ADD_INDY:
            return 2;
        case ADD_ABS:
        case ADD_ABSX:
        case ADD_ABSY:
        case ADD_IND:
            return 3;
        default:
            return 1;
    };
}

std::string_view Assembler::token_val(size_t idx) const {
    return tokens.at(idx).value;
}

std::optional<uint16_t> Assembler::resolve_numeric_token_value(const Token_T& t) const {
    size_t base = 10;
    switch(t.type) {
        case TOKEN_TYPE::NUMBER_DEC:
            break;
        case TOKEN_TYPE::NUMBER_HEX:
            base = 16;
            break;
        case TOKEN_TYPE::NUMBER_BIN:
            base = 2;
            break;
        default:
            return std::nullopt;
    };

    size_t value = 0;
    for (char c : t.value) {
        size_t digit;

        if (c >= '0' && c <= '9')
            digit = c - '0';
        else if (c >= 'a' && c <= 'f')
            digit = c - 'a' + 10;
        else if (c >= 'A' && c <= 'F')
            digit = c - 'A' + 10;
        else
            continue;

        if (digit >= base) {
            return std::nullopt;
        }

        value = value * base + digit;
    }

    return value;
}

std::optional<uint16_t> Assembler::resolve_symbol_address(const std::string_view& s) const {
    auto symbol_loc = symbol_table.find(s);
    if (symbol_loc == symbol_table.end()) {
        return std::nullopt;
    }
    return symbol_loc->second;
}

std::optional<uint16_t> Assembler::resolve_value(const Token_T& t) const {
    switch(t.type) {
        case TOKEN_TYPE::NUMBER_BIN:
        case TOKEN_TYPE::NUMBER_DEC:
        case TOKEN_TYPE::NUMBER_HEX:
            return resolve_numeric_token_value(t);

        case TOKEN_TYPE::IDENTIFIER:
            return resolve_symbol_address(t.value);

        default:
            return std::nullopt;
    };
}

void Assembler::emit_byte(uint8_t byte, uint16_t& current_pc) {
    if (!reset_vector_initialized) {
        reset_vector = current_pc;
        reset_vector_initialized = true;
    }

    uint32_t expected_pc = reset_vector + machine_code.size();

    if (current_pc > expected_pc) {
        uint32_t gap_size = current_pc - expected_pc;
        for (uint32_t i = 0; i < gap_size; ++i) {
            machine_code.push_back(0x00);
        }
    }

    machine_code.push_back(byte);
    ++current_pc;
}

bool Assembler::handle_label(const Statement_T& s, uint16_t pc, bool error_on_redefinition) {
    // Add the label to the symbol table.
    if (s.label.has_value()) {
        auto [it, inserted] = symbol_table.insert({token_val(s.label.value()), pc});
        if (!inserted) {
            if (error_on_redefinition) {
                Diagnostics::get().report({
                        DiagnosticSeverity::Error,
                        DiagnosticCode::AssemblerSymbolRedefinition,
                        {filename, s.line},
                        ("Symbol \"" + std::string(token_val(s.label.value())) + "\" is already defined.")
                        });
                return false;
            }
            if (it->second == pc) {
                return false;
            }
            it->second = pc;
            return true;
        }
    }
    return false;
}

void Assembler::handle_directive(const Statement_T& s, uint16_t& pc) {

    if (s.type != STATEMENT_TYPE::DIRECTIVE)
        return;

    if (!s.mnemonic.has_value()) {
        if (is_pass_1) {
            Diagnostics::get().report({
                    DiagnosticSeverity::Error,
                    DiagnosticCode::AssemblerNoMnemonic,
                    {filename, s.line},
                    "Directive is missing a mnemonic."
                    });
        }
        return;
    }

    switch(hash_djb2(token_val(s.mnemonic.value()))) {
        case "org"_hash:   dir_org(s, pc);  break;
        case "res"_hash:   dir_res(s, pc);  break;
        case "byte"_hash:  dir_byte(s, pc); break;
        case "word"_hash:  dir_word(s, pc); break;
        case "equ"_hash:   dir_equ(s);  break;
        case "fill"_hash:  dir_fill(s, pc); break;
        case "ascii"_hash: dir_ascii(s, pc); break;
        case "align"_hash: dir_align(s, pc); break;

        default:
            if (is_pass_1) {
                Diagnostics::get().report({
                    DiagnosticSeverity::Error,
                    DiagnosticCode::AssemblerUnknownDirective,
                    {filename, s.line},
                    ("Unknown directive: \"" + std::string(token_val(s.mnemonic.value())) + "\".")
                    });
            }
    };
}

void Assembler::dir_org(const Statement_T& s, uint16_t& pc) {

    if (!s.argument.has_value()) {
        if (is_pass_1) {
            Diagnostics::get().report({
                    DiagnosticSeverity::Error,
                    DiagnosticCode::AssemblerDirectiveError,
                    {filename, s.line},
                    ".org directive is missing an argument."
                    });
        }
        return;
    }

    const Argument_T& arg = s.argument.value();
    if (arg.end - arg.start != 1) {
        if (is_pass_1) {
            Diagnostics::get().report({
                    DiagnosticSeverity::Error,
                    DiagnosticCode::AssemblerDirectiveError,
                    {filename, s.line},
                    ".org expects exactly one argument."
                    });
        }
        return;
    }

    std::optional<uint16_t> value = resolve_value(tokens[arg.start]);
    if (!value.has_value()) {
        if (is_pass_1) {
            Diagnostics::get().report({
                    DiagnosticSeverity::Error,
                    DiagnosticCode::AssemblerDirectiveError,
                    {filename, s.line},
                    "Failed to resolve argument value."
                    });
        }
        return;
    }
    pc = value.value();
    return;
}

void Assembler::dir_res(const Statement_T& s, uint16_t& pc) {

    if (!s.argument.has_value()) {
        if (is_pass_1) {
            Diagnostics::get().report({
                    DiagnosticSeverity::Error,
                    DiagnosticCode::AssemblerDirectiveError,
                    {filename, s.line},
                    ".res directive is missing an argument."
                    });
        }
        return;
    }

    const Argument_T& arg = s.argument.value();
    if (arg.end - arg.start != 1) {
        if (is_pass_1) {
            Diagnostics::get().report({
                    DiagnosticSeverity::Error,
                    DiagnosticCode::AssemblerDirectiveError,
                    {filename, s.line},
                    ".res expects exactly one argument."
                    });
        }
        return;
    }

    std::optional<uint16_t> value = resolve_value(tokens[arg.start]);
    if (!value.has_value()) {
        if (is_pass_1) {
            Diagnostics::get().report({
                    DiagnosticSeverity::Error,
                    DiagnosticCode::AssemblerDirectiveError,
                    {filename, s.line},
                    "Failed to resolve argument value."
                    });
        }
        return;
    }
    pc += value.value();
    return;
}

void Assembler::dir_byte(const Statement_T& s, uint16_t& pc) {

    if (!s.argument.has_value()) {
        if (is_pass_1) {
            Diagnostics::get().report({
                    DiagnosticSeverity::Error,
                    DiagnosticCode::AssemblerDirectiveError,
                    {filename, s.line},
                    ".byte is missing an argument."
                    });
        }
        return;
    }

    const Argument_T& arg = s.argument.value();
    bool expects_num = true;
    for (size_t i = arg.start; i < arg.end; ++i) {
        if (expects_num) {
            TOKEN_TYPE tt = tokens[i].type;
            if (tt != TOKEN_TYPE::NUMBER_HEX && tt != TOKEN_TYPE::NUMBER_BIN
                    && tt != TOKEN_TYPE::NUMBER_DEC && tt != TOKEN_TYPE::IDENTIFIER) {
                if (is_pass_1) {
                    Diagnostics::get().report({
                            DiagnosticSeverity::Error,
                            DiagnosticCode::AssemblerDirectiveError,
                            {filename, s.line},
                            ".byte expects a numeric value or label."
                            });
                }
                return;
            }
            auto value = resolve_value(tokens[i]);
            if (!value.has_value()) {
                if (is_pass_1) {
                    Diagnostics::get().report({
                            DiagnosticSeverity::Error,
                            DiagnosticCode::AssemblerDirectiveError,
                            {filename, s.line},
                            "Failed to resolve value."
                            });
                }
                return;
            }
            emit_byte(value.value() & 0xFF, pc);
            expects_num = false;
            continue;
        }

        if (tokens[i].type != TOKEN_TYPE::COMMA) {
            if (is_pass_1) {
                Diagnostics::get().report({
                        DiagnosticSeverity::Error,
                        DiagnosticCode::AssemblerDirectiveError,
                        {filename, s.line},
                        ".byte expects a comma between values."
                        });
            }
            return;
        }
        expects_num = true;
    }
    return;
}

void Assembler::dir_word(const Statement_T& s, uint16_t& pc) {

    if (!s.argument.has_value()) {
        if (is_pass_1) {
            Diagnostics::get().report({
                    DiagnosticSeverity::Error,
                    DiagnosticCode::AssemblerDirectiveError,
                    {filename, s.line},
                    ".word is missing an argument."
                    });
        }
        return;
    }

    const Argument_T& arg = s.argument.value();
    bool expects_num = true;
    for (size_t i = arg.start; i < arg.end; ++i) {
        if (expects_num) {
            TOKEN_TYPE tt = tokens[i].type;
            if (tt != TOKEN_TYPE::NUMBER_HEX && tt != TOKEN_TYPE::NUMBER_BIN
                    && tt != TOKEN_TYPE::NUMBER_DEC && tt != TOKEN_TYPE::IDENTIFIER) {
                if (is_pass_1) {
                    Diagnostics::get().report({
                            DiagnosticSeverity::Error,
                            DiagnosticCode::AssemblerDirectiveError,
                            {filename, s.line},
                            ".word expects a numeric value or label."
                            });
                }
                return;
            }
            auto value = resolve_value(tokens[i]);
            if (!value.has_value()) {
                if (is_pass_1) {
                    Diagnostics::get().report({
                            DiagnosticSeverity::Error,
                            DiagnosticCode::AssemblerDirectiveError,
                            {filename, s.line},
                            "Failed to resolve value."
                            });
                }
                return;
            }
            emit_byte(value.value() & 0xFF, pc);
            emit_byte((value.value() >> 8) & 0xFF, pc);
            expects_num = false;
            continue;
        }

        if (tokens[i].type != TOKEN_TYPE::COMMA) {
            if (is_pass_1) {
                Diagnostics::get().report({
                        DiagnosticSeverity::Error,
                        DiagnosticCode::AssemblerDirectiveError,
                        {filename, s.line},
                        ".word expects a comma between values."
                        });
            }
            return;
        }
        expects_num = true;
    }
    return;
}

void Assembler::dir_equ(const Statement_T& s) {

    if (!s.argument.has_value()) {
        if (is_pass_1) {
            Diagnostics::get().report({
                    DiagnosticSeverity::Error,
                    DiagnosticCode::AssemblerDirectiveError,
                    {filename, s.line},
                    ".equ is missing an argument."
                    });
        }
        return;
    }

    const Argument_T& arg = s.argument.value();
    if (arg.end - arg.start != 2) {
        if (is_pass_1) {
            Diagnostics::get().report({
                    DiagnosticSeverity::Error,
                    DiagnosticCode::AssemblerDirectiveError,
                    {filename, s.line},
                    ".equ expects exactly two arguments."
                    });
        }
        return;
    }

    if (tokens[arg.start].type != TOKEN_TYPE::IDENTIFIER) {
        if (is_pass_1) {
            Diagnostics::get().report({
                    DiagnosticSeverity::Error,
                    DiagnosticCode::AssemblerDirectiveError,
                    {filename, s.line},
                    ".equ expects the first argument to be an identifier."
                    });
        }
        return;
    }

    const std::string_view& symbol = token_val(arg.start);
    auto value = resolve_value(tokens[arg.start + 1]);
    if (!value.has_value()) {
        if (is_pass_1) {
            Diagnostics::get().report({
                    DiagnosticSeverity::Error,
                    DiagnosticCode::AssemblerDirectiveError,
                    {filename, s.line},
                    "Failed to resolve argument value."
                    });
        }
        return;
    }

    auto symbol_loc = symbol_table.find(symbol);
    if (symbol_loc != symbol_table.end()) {
        if (value.value() != symbol_loc->second) {
            if (is_pass_1) {
                Diagnostics::get().report({
                        DiagnosticSeverity::Error,
                        DiagnosticCode::AssemblerDirectiveError,
                        {filename, s.line},
                        "Symbol \"" + std::string(token_val(arg.start)) + "\" is already defined."
                        });
            }
        }
        return;
    }

    bool inserted = symbol_table.insert({symbol, value.value()}).second;
    if (!inserted) {
        if (is_pass_1) {
            Diagnostics::get().report({
                    DiagnosticSeverity::Error,
                    DiagnosticCode::AssemblerDirectiveError,
                    {filename, s.line},
                    "Failed to assign value to symbol."
                    });
        }
        return;
    }
}

void Assembler::dir_fill(const Statement_T& s, uint16_t& pc) {

    if (!s.argument.has_value()) {
        if (is_pass_1) {
            Diagnostics::get().report({
                    DiagnosticSeverity::Error,
                    DiagnosticCode::AssemblerDirectiveError,
                    {filename, s.line},
                    ".fill is missing an argument."
                    });
        }
        return;
    }
    auto arg = s.argument.value();

    if (arg.end - arg.start != 3) {
        if (is_pass_1) {
            Diagnostics::get().report({
                    DiagnosticSeverity::Error,
                    DiagnosticCode::AssemblerDirectiveError,
                    {filename, s.line},
                    "Failed to resolve argument."
                    });
        }
        return;
    }

    size_t idx = arg.start;
    auto range = resolve_value(tokens[idx]);
    if (!range.has_value()) {
        if (is_pass_1) {
            Diagnostics::get().report({
                    DiagnosticSeverity::Error,
                    DiagnosticCode::AssemblerDirectiveError,
                    {filename, s.line},
                    "Failed to resolve first parameter value."
                    });
        }
        return;
    }
    ++idx;

    if (tokens[idx].type != TOKEN_TYPE::COMMA) {
        if (is_pass_1) {
            Diagnostics::get().report({
                    DiagnosticSeverity::Error,
                    DiagnosticCode::AssemblerDirectiveError,
                    {filename, s.line},
                    "Expected a comma between parameter values."
                    });
        }
        return;
    }
    ++idx;

    auto value = resolve_value(tokens[idx]);
    if (!value.has_value()) {
        if (is_pass_1) {
            Diagnostics::get().report({
                    DiagnosticSeverity::Error,
                    DiagnosticCode::AssemblerDirectiveError,
                    {filename, s.line},
                    "Failed to resolve second parameter value."
                    });
        }
        return;
    }

    for (uint16_t i = 0; i < range.value(); ++i) {
        emit_byte(value.value(), pc);
    }
}

void Assembler::dir_ascii(const Statement_T& s, uint16_t& pc) {

    if (!s.argument.has_value()) {
        if (is_pass_1) {
            Diagnostics::get().report({
                    DiagnosticSeverity::Error,
                    DiagnosticCode::AssemblerDirectiveError,
                    {filename, s.line},
                    ".ascii is missing an argument."
                    });
        }
        return;
    }
    auto arg = s.argument.value();

    size_t idx = arg.start;
    if (tokens[idx].type != TOKEN_TYPE::STRING_LITERAL) {
        if (is_pass_1) {
            Diagnostics::get().report({
                    DiagnosticSeverity::Error,
                    DiagnosticCode::AssemblerDirectiveError,
                    {filename, s.line},
                    ".ascii expects a string literal."
                    });
        }
        return;
    }

    for (auto c : tokens[idx].value) {
        if (c != '\"') {
            emit_byte(c, pc);
        }
    }

}

void Assembler::dir_align(const Statement_T& s, uint16_t& pc) {

    if (!s.argument.has_value()) {
        if (is_pass_1) {
            Diagnostics::get().report({
                    DiagnosticSeverity::Error,
                    DiagnosticCode::AssemblerDirectiveError,
                    {filename, s.line},
                    ".align is missing an argument."
                    });
        }
        return;
    }
    auto arg = s.argument.value();

    if (arg.end - arg.start != 1) {
        if (is_pass_1) {
            Diagnostics::get().report({
                    DiagnosticSeverity::Error,
                    DiagnosticCode::AssemblerDirectiveError,
                    {filename, s.line},
                    ".align expects exactly one argument."
                    });
        }
        return;
    }

    auto value = resolve_value(tokens[arg.start]);
    if (!value.has_value()) {
        if (is_pass_1) {
            Diagnostics::get().report({
                    DiagnosticSeverity::Error,
                    DiagnosticCode::AssemblerDirectiveError,
                    {filename, s.line},
                    "Failed to resolve argument value."
                    });
        }
        return;
    }

    uint16_t alignment = value.value();
    if (alignment == 0 || (alignment & (alignment - 1)) != 0) {
        if (is_pass_1) {
            Diagnostics::get().report({
                    DiagnosticSeverity::Error,
                    DiagnosticCode::AssemblerDirectiveError,
                    {filename, s.line},
                    "Argument must be a power of 2."
                    });
        }
        return;
    }

    uint16_t remainder = pc % alignment;
    uint16_t padding = (remainder == 0) ? 0 : (alignment - remainder);
    for (uint16_t i = 0; i < padding; ++i) {
        emit_byte(0x00, pc);
    }
};

std::optional<uint16_t> Assembler::resolve_ins_arg(const Argument_T& arg, ADDRESSING_MODE addr_mode) const {

    switch (addr_mode) {

        case ADD_ABS: return resolve_arg_ABS(arg);
        case ADD_ABSX: return resolve_arg_ABSX(arg);
        case ADD_ABSY: return resolve_arg_ABSY(arg);
        case ADD_IMM: return resolve_arg_IMM(arg);
        case ADD_IND: return resolve_arg_IND(arg);
        case ADD_INDX: return resolve_arg_INDX(arg);
        case ADD_INDY: return resolve_arg_INDY(arg);
        case ADD_REL: return resolve_arg_REL(arg);
        case ADD_ZPG: return resolve_arg_ZPG(arg);
        case ADD_ZPGX: return resolve_arg_ZPGX(arg);
        case ADD_ZPGY: return resolve_arg_ZPGY(arg);

        default:
            return std::nullopt;
    }
}

std::optional<uint16_t> Assembler::resolve_arg_ABS(const Argument_T& arg) const {

    if (arg.end - arg.start != 1)
        return std::nullopt;

    return resolve_value(tokens[arg.start]);
}

std::optional<uint16_t> Assembler::resolve_arg_ABSX(const Argument_T& arg) const {

    if (arg.end - arg.start != 3)
        return std::nullopt;

    auto value = resolve_value(tokens[arg.start]);

    if (tokens[arg.start + 1].type != TOKEN_TYPE::COMMA)
        return std::nullopt;

    const std::string_view& xval = token_val(arg.start + 2);
    if (xval != "x" && xval != "X")
        return std::nullopt;

    return value;
}

std::optional<uint16_t> Assembler::resolve_arg_ABSY(const Argument_T& arg) const {

    if (arg.end - arg.start != 3)
        return std::nullopt;

    auto value = resolve_value(tokens[arg.start]);

    if (tokens[arg.start + 1].type != TOKEN_TYPE::COMMA)
        return std::nullopt;

    const std::string_view& yval = token_val(arg.start + 2);
    if (yval != "y" && yval != "Y")
        return std::nullopt;

    return value;
}

std::optional<uint16_t> Assembler::resolve_arg_IMM(const Argument_T& arg) const {

    if (arg.end - arg.start != 2)
        return std::nullopt;

    if (tokens[arg.start].type != TOKEN_TYPE::HASH)
        return std::nullopt;

    return resolve_value(tokens[arg.start + 1]);
}

std::optional<uint16_t> Assembler::resolve_arg_IND(const Argument_T& arg) const {

    if (arg.end - arg.start != 3)
        return std::nullopt;

    size_t idx = arg.start;
    if (tokens[idx].type != TOKEN_TYPE::LPAREN)
        return std::nullopt;
    ++idx;

    auto value = resolve_value(tokens[idx]);
    ++idx;

    if (tokens[idx].type != TOKEN_TYPE::RPAREN)
        return std::nullopt;

    return value;
}

std::optional<uint16_t> Assembler::resolve_arg_INDX(const Argument_T& arg) const {

    if (arg.end - arg.start != 5)
        return std::nullopt;

    size_t idx = arg.start;
    if (tokens[idx].type != TOKEN_TYPE::LPAREN)
        return std::nullopt;
    ++idx;

    auto value = resolve_value(tokens[idx]);
    ++idx;

    if (tokens[idx].type != TOKEN_TYPE::COMMA)
        return std::nullopt;
    ++idx;

    if (tokens[idx].type != TOKEN_TYPE::IDENTIFIER)
        return std::nullopt;
    const std::string_view& xval = token_val(idx);
    if (xval != "x" && xval != "X")
        return std::nullopt;
    ++idx;

    if (tokens[idx].type != TOKEN_TYPE::RPAREN)
        return std::nullopt;

    return value;
}

std::optional<uint16_t> Assembler::resolve_arg_INDY(const Argument_T& arg) const {

    if (arg.end - arg.start != 5)
        return std::nullopt;

    size_t idx = arg.start;
    if (tokens[idx].type != TOKEN_TYPE::LPAREN)
        return std::nullopt;
    ++idx;

    auto value = resolve_value(tokens[idx]);
    ++idx;

    if (tokens[idx].type != TOKEN_TYPE::RPAREN)
        return std::nullopt;
    ++idx;

    if (tokens[idx].type != TOKEN_TYPE::COMMA)
        return std::nullopt;
    ++idx;

    if (tokens[idx].type != TOKEN_TYPE::IDENTIFIER)
        return std::nullopt;
    const std::string_view& yval = token_val(idx);
    if (yval != "y" && yval != "Y")
        return std::nullopt;
    ++idx;

    return value;
}

std::optional<uint16_t> Assembler::resolve_arg_REL(const Argument_T& arg) const {

    if (arg.end - arg.start != 1)
        return std::nullopt;

    return resolve_value(tokens[arg.start]);
}

std::optional<uint16_t> Assembler::resolve_arg_ZPG(const Argument_T& arg) const {

    if (arg.end - arg.start != 1)
        return std::nullopt;

    return resolve_value(tokens[arg.start]);
}

std::optional<uint16_t> Assembler::resolve_arg_ZPGX(const Argument_T& arg) const {

    if (arg.end - arg.start != 3)
        return std::nullopt;

    auto value = resolve_value(tokens[arg.start]);

    if (tokens[arg.start + 1].type != TOKEN_TYPE::COMMA)
        return std::nullopt;

    const std::string_view& xval = token_val(arg.start + 2);
    if (xval != "x" && xval != "X")
        return std::nullopt;

    return value;
}

std::optional<uint16_t> Assembler::resolve_arg_ZPGY(const Argument_T& arg) const {

    if (arg.end - arg.start != 3)
        return std::nullopt;

    auto value = resolve_value(tokens[arg.start]);

    if (tokens[arg.start + 1].type != TOKEN_TYPE::COMMA)
        return std::nullopt;

    const std::string_view& yval = token_val(arg.start + 2);
    if (yval != "y" && yval != "Y")
        return std::nullopt;

    return value;
}

