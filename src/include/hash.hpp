#ifndef HASH_HPP
#define HASH_HPP

#include <string_view>
#include <cstdint>

constexpr uint32_t hash_djb2(std::string_view str) {
    uint32_t hash = 5381;
    for (char c : str) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash;
}

constexpr uint32_t operator""_hash(const char* str, size_t len) {
    return hash_djb2(std::string_view(str, len));
}

#endif
