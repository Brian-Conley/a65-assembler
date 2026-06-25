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
    std::string outFileName = "out.bin";

    Lexer lex(path);
    Parser par(lex.tokenize(), path);
    Assembler ass(lex.tokenize(), par.parse());
    const auto code = ass.assemble();

    if (Diagnostics::get().hasErrors()) {
        Diagnostics::get().printTerminationMessage();
        exit(static_cast<int>(DiagnosticCode::AssemblyFailed));
    }

    std::ofstream outfile(outFileName, std::ios::out | std::ios::binary);
    if (!outfile) {
        Diagnostics::get().report({
                DiagnosticSeverity::Fatal,
                DiagnosticCode::FileSystemError,
                {path, 0},
                "Failed to open file \"" + outFileName + "\"."
                });
    }
    outfile.write(reinterpret_cast<const char*>(code.data()), code.size());
    if (!outfile) {
        Diagnostics::get().report({
                DiagnosticSeverity::Fatal,
                DiagnosticCode::FileSystemError,
                {path, 0},
                "Failed to write to file \"" + outFileName + "\"."
                });
    }

    return 0;
}
