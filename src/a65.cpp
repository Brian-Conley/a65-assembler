#include <fstream>
#include <iostream>
#include <string>
#include "include/diagnostics.hpp"
#include "include/lexer.hpp"
#include "include/parser.hpp"
#include "include/assembler.hpp"

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " file.a65\n";
        return 1;
    }
    std::string path(argv[1]);

    Lexer lex(path);
    Parser par(lex.tokenize());
    Assembler ass(lex.tokenize(), par.parse());
    const auto code = ass.assemble();

    std::ofstream outfile("out.bin", std::ios::out | std::ios::binary);
    if (!outfile) {
        std::cerr << "Error: Could not open file \"out.bin\".\n";
        return 1;
    }
    outfile.write(reinterpret_cast<const char*>(code.data()), code.size());
    if (!outfile) {
        std::cerr << "Error: Failed to write to \"out.bin\".\n";
        return 1;
    }

    return 0;
}
