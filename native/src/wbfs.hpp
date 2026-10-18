#pragma once
#include <algorithm>
#include <array>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <vector>

inline uint32_t be32(const uint8_t* p) {
    return uint32_t(p[0]) << 24 | uint32_t(p[1]) << 16 | uint32_t(p[2]) << 8 | p[3];
}

// Read-only, single-file / first-disc WBFS reader. No partition decryption yet.
class Wbfs {
    std::ifstream file;
    uint64_t length = 0, blockSize = 0;
    std::vector<uint16_t> blocks;
    void physical(uint64_t offset, void* dest, size_t count) {
        if (offset > length || count > length - offset) throw std::runtime_error("Truncated WBFS or split image (split files are not supported yet)");
        file.clear();
        file.seekg(static_cast<std::streamoff>(offset));
        if (!file.read(static_cast<char*>(dest), static_cast<std::streamsize>(count))) throw std::runtime_error("Disc read failed");
    }
public:
    static constexpr uint64_t capacity = uint64_t(143432) * 2 * 0x8000;
    explicit Wbfs(const std::filesystem::path& path) : file(path, std::ios::binary) {
        if (!file) throw std::runtime_error("Cannot open WBFS");
        length = std::filesystem::file_size(path);
        std::array<uint8_t, 12> header{};
        physical(0, header.data(), header.size());
        if (std::memcmp(header.data(), "WBFS", 4)) throw std::runtime_error("Not a WBFS image");
        if (header[8] < 9 || header[8] > 20 || header[9] < 15 || header[9] > 30 || header[9] < header[8]) throw std::runtime_error("Invalid sector sizes");
        uint64_t sector = uint64_t(1) << header[8];
        blockSize = uint64_t(1) << header[9];
        if (uint64_t(be32(header.data() + 4)) * sector != length) throw std::runtime_error("WBFS size mismatch; split or truncated image");
        std::vector<uint8_t> slots(static_cast<size_t>(sector - 12));
        physical(12, slots.data(), slots.size());
        if (slots[0] == 0 || std::any_of(slots.begin() + 1, slots.end(), [](auto x){ return x != 0; })) throw std::runtime_error("Only first-slot, single-disc WBFS files are supported");
        size_t count = static_cast<size_t>((capacity + blockSize - 1) / blockSize);
        std::vector<uint8_t> table(count * 2);
        physical(sector + 256, table.data(), table.size());
        for (size_t i = 0; i < count; ++i) {
            uint16_t block = uint16_t(table[i*2]) << 8 | table[i*2+1];
            if (block && uint64_t(block) * blockSize >= length) throw std::runtime_error("Invalid WBFS block mapping");
            blocks.push_back(block);
        }
    }
    std::vector<uint8_t> read(uint64_t offset, size_t count) {
        if (offset > capacity || count > capacity - offset) throw std::runtime_error("Read outside logical disc");
        if (count > 16 * 1024 * 1024) throw std::runtime_error("Read exceeds 16 MiB safety limit");
        std::vector<uint8_t> out(count, 0);
        size_t done = 0;
        while (done < count) {
            uint64_t logical = offset + done;
            auto block = blocks.at(static_cast<size_t>(logical / blockSize));
            size_t chunk = static_cast<size_t>(std::min<uint64_t>(count - done, blockSize - logical % blockSize));
            if (block) physical(uint64_t(block) * blockSize + logical % blockSize, out.data() + done, chunk);
            done += chunk;
        }
        return out;
    }
};
