#include "include/createBuffer.hpp"
#include <fstream>

std::string create_file_buffer(const std::string& path) {
    std::ifstream file(path);
    file.seekg(0, std::ios::end);
    std::string buffer(file.tellg(), '\0');
    file.seekg(0);
    file.read(buffer.data(), buffer.size());
    return buffer;
}
