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
        /**
         * @brief The intended constructor for the Assembler.
         *
         * @param tokens The vector of tokens produced by the Lexer.
         * @param statements The vector of statements produced by the Parser.
         */
        Assembler(const std::vector<Token_T>& tokens,
                const std::vector<Statement_T>& statements);

        /**
         * @brief Turn the statements into machine code.
         *
         * @return A const reference vector to the machine code bytes.
         */
        const std::vector<uint8_t>& assemble();

    private:
        Assembler() = delete;

        /** @name Data Buffers */
        ///@{
        const std::vector<Token_T>& tokens;         /**< All of the tokens. */
        const std::vector<Statement_T>& statements; /**< All of the statements. */
        std::vector<uint8_t> machine_code;          /**< Generated machine code. */
        ///@}

        /** @name Internal State */
        ///@{
        std::unordered_map<std::string_view, uint16_t> symbol_table; /**< Pairs string values to 16-bit addresses. */
        bool assembled;                                              /**< Flag to prevent duplicate work from multiple assemble() calls. */
        const uint16_t reset_vector;                                 /**< The expected starting address of the program. */
        ///@}

        /**
         * @brief Find the instruction type specified by the token.
         *
         * @retval nullopt No instruction type is associated with the token.
         */
        std::optional<INSTRUCTION_TYPE> determine_instruction_type(const Token_T& t) const;

        /**
         * @brief Derive the addressing mode from the instruction and its argument.
         */
        ADDRESSING_MODE determine_addressing_mode(INSTRUCTION_TYPE ins, const std::optional<Argument_T>& arg) const;

        /**
         * @brief Determine how many bytes an instruction will take based on the addressing mode.
         */
        size_t get_mode_size(ADDRESSING_MODE mode) const;

        /**
         * @brief Write a byte to memory and increment the PC.
         */
        void emit_byte(uint8_t byte, uint16_t& current_pc);

        /**
         * @brief Get tokens[idx].value
         */
        std::string_view token_val(size_t idx) const;

        /**
         * @brief Convert a numeric token value into a usable number.
         *
         * @retval nullopt Conversion failed
         */
        std::optional<uint16_t> resolve_numeric_token_value(const Token_T& t) const;

        /**
         * @brief Get the address of a symbol from the symbol table.
         *
         * @retval nullopt Unable to retrieve the symbol's address.
         */
        std::optional<uint16_t> resolve_symbol_address(const std::string_view& s) const;

        /**
         * @brief Simplifies resolving values by handling the logic for distinguishing numeric tokens and symbol lookups.
         */
        std::optional<uint16_t> resolve_value(const Token_T& t) const;

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

        /**
         * @brief Dispatches control flow to the appropriate directive handler.
         */
        void handle_directive(const Statement_T& s, uint16_t& pc);

        /** @name Directive Handlers */
        ///@{
        void dir_org(const Statement_T& s, uint16_t& pc);
        void dir_res(const Statement_T& s, uint16_t& pc);
        void dir_byte(const Statement_T& s, uint16_t& pc);
        void dir_word(const Statement_T& s, uint16_t& pc);
        void dir_equ(const Statement_T& s);
        void dir_fill(const Statement_T& s, uint16_t& pc);
        void dir_ascii(const Statement_T& s, uint16_t& pc);
        void dir_align(const Statement_T& s, uint16_t& pc);
        ///@}

        /**
         * @brief Dispatches control flow to the appropriate argument resolver.
         */
        std::optional<uint16_t> resolve_ins_arg(const Argument_T& arg, ADDRESSING_MODE addr_mode) const;

        /** @name Argument Resolvers */
        ///@{
        std::optional<uint16_t> resolve_arg_ABS(const Argument_T& arg) const;
        std::optional<uint16_t> resolve_arg_ABSX(const Argument_T& arg) const;
        std::optional<uint16_t> resolve_arg_ABSY(const Argument_T& arg) const;
        std::optional<uint16_t> resolve_arg_IMM(const Argument_T& arg) const;
        std::optional<uint16_t> resolve_arg_IND(const Argument_T& arg) const;
        std::optional<uint16_t> resolve_arg_INDX(const Argument_T& arg) const;
        std::optional<uint16_t> resolve_arg_INDY(const Argument_T& arg) const;
        std::optional<uint16_t> resolve_arg_REL(const Argument_T& arg) const;
        std::optional<uint16_t> resolve_arg_ZPG(const Argument_T& arg) const;
        std::optional<uint16_t> resolve_arg_ZPGX(const Argument_T& arg) const;
        std::optional<uint16_t> resolve_arg_ZPGY(const Argument_T& arg) const;
        ///@}
};

#endif
