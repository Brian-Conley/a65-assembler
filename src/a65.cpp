#include <iostream>
#include <string>
#include "include/lexer.hpp"
#include "include/token.hpp"

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " file.a65\n";
        return 1;
    }

    std::string path(argv[1]);
    Lexer lex(path);

    size_t token_count = 0;
    size_t line_count = 0;
    for (Token_T t : lex.tokenize()) {
        ++token_count;

        if (t.type == TOKEN_TYPE::NEWLINE) {
            std::cout << "<NEWLINE>\n";
            ++line_count;
            continue;
        }

        std::cout << "<" << type_to_str(t.type) << ": "
            << t.value << "> ";
    }

    std::cout << "Processed " << line_count << " lines containing "
        << token_count << " tokens.\n";

    return 0;
}
