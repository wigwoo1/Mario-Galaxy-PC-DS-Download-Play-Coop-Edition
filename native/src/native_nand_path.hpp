#pragma once
#include <filesystem>
#include <stdexcept>
#include <string>
namespace Native {
// Resolve Wii NAND names beneath the explicitly selected host save root.
inline std::filesystem::path nandFilePath(const std::filesystem::path& root, const char* name) {
    if (!name || !*name) throw std::invalid_argument("Empty NAND name");
    const std::string text(name);
    if (text.size() >= 64 || text.find('\\') != std::string::npos || text.find(':') != std::string::npos)
        throw std::invalid_argument("Invalid NAND name");
    std::filesystem::path relative;
    size_t begin = text[0] == '/' ? 1 : 0;
    while (begin <= text.size()) {
        const size_t end = text.find('/', begin);
        const auto part = text.substr(begin, end == std::string::npos ? end : end - begin);
        if (part.empty() || part == "." || part == ".." || part.back() == '.' || part.back() == ' ')
            throw std::invalid_argument("Invalid NAND component");
        relative /= std::filesystem::u8path(part);
        if (end == std::string::npos) break;
        begin = end + 1;
    }
    auto base = std::filesystem::weakly_canonical(root);
    auto result = std::filesystem::weakly_canonical(base / relative);
    auto b = base.begin(), r = result.begin();
    for (; b != base.end(); ++b, ++r)
        if (r == result.end() || *b != *r) throw std::invalid_argument("NAND path escapes save root");
    if (r == result.end()) throw std::invalid_argument("NAND file names root");
    return result;
}
}
