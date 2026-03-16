#include <iostream>
#include <string>
#include "include/lexer.hpp"

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " file.a65\n";
        return 1;
    }

    std::string path(argv[1]);
    Lexer lex(path);

    size_t count = 0;
    for (auto t : lex.tokenize()) {
        // std::cout << "token: " << t.value << '\n';
        count += 1;
    }

    std::cout << "Processed " << count << " tokens\n";

    return 0;
}
