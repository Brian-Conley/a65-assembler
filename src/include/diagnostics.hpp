#ifndef DIAGNOSTICS_HPP
#define DIAGNOSTICS_HPP

#include <string>

enum class DiagnosticSeverity {
    Warning,
    Error,
    Fatal
};

enum class DiagnosticCode {
    FileSystemError,

    // Lexer
    LexerError,
    LexerUnknownChar,
    LexerUnknownSymbol,

    ParserError,
    ParserUnknownToken,
    ParserUnexpectedToken,
    ParserOutOfBounds,

    AssemblerError,
    AssemblerNoMnemonic,
    AssemblerUnknownInstruction,
    AssemblerUnknownDirective,
    AssemblerBadAddressMode,
    AssemblerUnresolvedArg,
    AssemblerJumpTooLarge,
    AssemblerUnresolvedAddressMode,
    AssemblerSymbolRedefinition,
    AssemblerDirectiveError,
    AssemblyFailed
};

struct SourceLocation {
    std::string_view filename;
    size_t line = 0;
};

struct Diagnostic {
    DiagnosticSeverity severity;
    DiagnosticCode code;
    SourceLocation location;
    std::string message;
};

class Diagnostics {
public:
    static Diagnostics& get();

    void report(const Diagnostic& d);

    bool hasErrors() const;

    void printTerminationMessage() const;

    size_t getErrorCount() const;

    size_t getWarningCount() const;

private:
    Diagnostics() = default;

    size_t errorCount;
    size_t warningCount;

    void printDiagnostic(const Diagnostic& d, const char* color, const char* severity) const;
    std::string code_to_str(DiagnosticCode c) const;

    void warn(const Diagnostic& d);

    void error(const Diagnostic& d);

    void fatal(const Diagnostic& d);

};

#endif
