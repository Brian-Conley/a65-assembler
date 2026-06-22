#ifndef STATEMENT_HPP
#define STATEMENT_HPP

#include <optional>

/** * Represents the syntactic category of a parsed statement. */
enum class STATEMENT_TYPE {
    INSTRUCTION, /**< Standard instruction containing an opcode/mnemonic. */
    DIRECTIVE,   /**< Assembler directive (where the mnemonic starts with '.'). */
    LABEL_ONLY,  /**< Standalone label definition with no accompanying instruction. */
};

/** * Index range of a statement argument in the form [start, end). */
struct Argument_T {
    size_t start; /**< Inclusive index of the first token. */
    size_t end;   /**< Exclusive index after the final token. */
};

/** * A bundle of token indices that comprise a single parsed statement. */
struct Statement_T {
    STATEMENT_TYPE type;                /**< The syntactic category of this statement. */
    size_t line;                        /**< Line number of the source */
    std::optional<size_t> label;        /**< (optional) Label associated with the statement. */
    std::optional<size_t> mnemonic;     /**< (optional) The keyword of the statement. */
    std::optional<Argument_T> argument; /**< (optional) The argument provided for the keyword. */
};

#endif
