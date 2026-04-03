#ifndef TYPE_PRINTERS_HPP
#define TYPE_PRINTERS_HPP

#include "token.hpp"
#include "statement.hpp"
#include <vector>

const std::string token_to_str(const Token_T& t);

const std::string statement_to_str(const std::vector<Token_T>& tokens, const Statement_T& s);

#endif
