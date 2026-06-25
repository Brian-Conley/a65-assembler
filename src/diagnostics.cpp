#include "include/diagnostics.hpp"
#include <cstdlib>
#include <iostream>

Diagnostics& Diagnostics::get() {
    static Diagnostics instance;
    return instance;
}

void Diagnostics::report(const Diagnostic& d) {
    switch (d.severity) {
        case DiagnosticSeverity::Warning:  warn(d); break;
        case DiagnosticSeverity::Error:   error(d); break;
        case DiagnosticSeverity::Fatal:   fatal(d); break;
    };
}

bool Diagnostics::hasErrors() const { return (warningCount > 0) || (errorCount > 0); }

size_t Diagnostics::getErrorCount() const { return errorCount; }

size_t Diagnostics::getWarningCount() const { return warningCount; }

void Diagnostics::printDiagnostic(const Diagnostic& d, const char* color, const char* severity) const {
    std::cerr << color
              << '[' << severity << "] "
              << code_to_str(d.code) << "\033[0m in "
              << d.location.filename
              << " (line " << d.location.line << "): "
              << d.message << '\n';
}

std::string Diagnostics::code_to_str(DiagnosticCode c) const {
    switch (c) {
        case DiagnosticCode::FileSystemError:       return "FileSystemError";
        case DiagnosticCode::LexerError:            return "LexerError";
        case DiagnosticCode::LexerUnknownChar:      return "LexerUnknownChar";
        case DiagnosticCode::LexerUnknownSymbol:    return "LexerUnknownSymbol";
        case DiagnosticCode::ParserError:           return "ParserError";
        case DiagnosticCode::ParserUnknownToken:    return "ParserUnknownToken";
        case DiagnosticCode::ParserUnexpectedToken: return "ParserUnexpectedToken";
        case DiagnosticCode::ParserOutOfBounds:     return "ParserOutOfBounds";
        case DiagnosticCode::AssemblerError:        return "AssemblerError";
        default:                                    return "UnknownError";
    };
}

void Diagnostics::warn(const Diagnostic& d) {
    printDiagnostic(d, "\033[33m", "W");
    ++warningCount;
}

void Diagnostics::error(const Diagnostic& d) {
    printDiagnostic(d, "\033[31m", "E");
    ++errorCount;
}

void Diagnostics::fatal(const Diagnostic& d) {
    printDiagnostic(d, "\033[31m", "F");
    ++errorCount;

    std::cerr << "\nTerminated with \033[33m"
              << warningCount << "\033[0m warnings "
              << "and \033[31m" << errorCount << "\033[0m errors.\n";
    exit(static_cast<int>(d.code));
}

