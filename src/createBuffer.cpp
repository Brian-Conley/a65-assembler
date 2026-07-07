#include "include/createBuffer.hpp"
#include "include/diagnostics.hpp"
#include <fstream>

std::string create_file_buffer(const std::string& path) {
    std::ifstream file(path);

    if (!file) {
        Diagnostics::get().report({
                DiagnosticSeverity::Fatal,
                DiagnosticCode::FileSystemError,
                {path, 0},
                "Failed to open file."
                });
    }

    file.seekg(0, std::ios::end);
    std::string buffer(file.tellg(), '\0');
    file.seekg(0);
    file.read(buffer.data(), buffer.size());
    file.close();
    return buffer;
}
