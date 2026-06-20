#ifndef ASSEMBLER_HPP
#define ASSEMBLER_HPP

#include "opcodes.hpp"
#include "token.hpp"
#include "statement.hpp"
#include <cstdint>
#include <optional>
#include <string_view>
#include <unordered_map>
#include <vector>

class Assembler {
    public:
        Assembler(const std::vector<Token_T>& tokens,
                const std::vector<Statement_T>& statements);

        const std::vector<uint8_t>& assemble();

    private:
        Assembler() = delete;

        const std::vector<Token_T>& tokens;
        const std::vector<Statement_T>& statements;

        std::unordered_map<std::string_view, uint16_t> symbol_table;
        std::vector<uint8_t> machine_code;
        bool assembled;
        const uint16_t reset_vector;

        std::optional<INSTRUCTION_TYPE> determine_instruction_type(const Token_T& t) const;
        ADDRESSING_MODE determine_addressing_mode(INSTRUCTION_TYPE ins, const std::optional<Argument_T>& arg) const;
        size_t get_mode_size(ADDRESSING_MODE mode) const;
        uint16_t resolve_numeric_token_value(const Token_T& t) const;
        std::optional<uint16_t> resolve_symbol_address(const std::string_view& s);
        std::string_view token_val(size_t idx) const;
        void emit_byte(uint8_t byte, uint16_t& current_pc);

        /**
         * @brief Inserts or overwrites a label's address in the symbol table.
         *
         * Checks for a label within a statement and attempts to insert it. If the symbol
         * already exists and error_on_redefinition is false, the value is updated.
         *
         * Both a new symbol insertion and the absence of a label will return false.
         *
         * @param s The statement potentially containing a label.
         * @param pc The current value of the program counter.
         * @param error_on_redefinition True to throw an error if the symbol exists (typically Pass 1).
         *
         * @retval true  The symbol already existed and its address shifted/changed.
         * @retval false The symbol was newly added, stayed the same, or no label was present.
         */
        bool handle_label(const Statement_T& s, uint16_t pc, bool error_on_redefinition=false);

        void handle_directive(const Statement_T& s, uint16_t& pc);

        void dir_org(const Statement_T& s, uint16_t& pc);
        void dir_res(const Statement_T& s, uint16_t& pc);
        void dir_byte(const Statement_T& s, uint16_t& pc);
        void dir_word(const Statement_T& s, uint16_t& pc);
        void dir_equ(const Statement_T& s, uint16_t& pc);

        std::optional<uint16_t> resolve_ins_arg(const Argument_T& arg, ADDRESSING_MODE addr_mode);
        std::optional<uint16_t> resolve_arg_ABS(const Argument_T& arg);
        std::optional<uint16_t> resolve_arg_ABSX(const Argument_T& arg);
        std::optional<uint16_t> resolve_arg_ABSY(const Argument_T& arg);
        std::optional<uint16_t> resolve_arg_IMM(const Argument_T& arg);
        std::optional<uint16_t> resolve_arg_IND(const Argument_T& arg);
        std::optional<uint16_t> resolve_arg_INDX(const Argument_T& arg);
        std::optional<uint16_t> resolve_arg_INDY(const Argument_T& arg);
        std::optional<uint16_t> resolve_arg_REL(const Argument_T& arg);
        std::optional<uint16_t> resolve_arg_ZPG(const Argument_T& arg);
        std::optional<uint16_t> resolve_arg_ZPGX(const Argument_T& arg);
        std::optional<uint16_t> resolve_arg_ZPGY(const Argument_T& arg);

};

#endif
