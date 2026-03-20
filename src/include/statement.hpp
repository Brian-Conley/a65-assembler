#ifndef STATEMENT_HPP
#define STATEMENT_HPP

#include <optional>
#include <string>

enum class STATEMENT_TYPE {
    INSTRUCTION,
    DIRECTIVE,
    LABEL_ONLY,
};

struct Argument_T {
    size_t start;
    size_t end;
};

struct Statement_T {
    STATEMENT_TYPE type;
    std::optional<size_t> label;
    std::optional<size_t> mnemonic;
    std::optional<Argument_T> argument;
};

#endif
