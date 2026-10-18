#pragma once
#include "wbfs.hpp"
#include <chrono>

inline void wbfsTests() {
    const size_t block = 1 << 20;
    auto dir = std::filesystem::temp_directory_path() / ("galaxy-wbfs-test-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    if (!std::filesystem::create_directory(dir)) throw std::runtime_error("Cannot create isolated test directory");
    struct Cleanup { std::filesystem::path dir; ~Cleanup() { std::error_code e; std::filesystem::remove_all(dir, e); } } cleanup{dir};
    auto path = dir / "fixture.wbfs";
    std::vector<uint8_t> data(4 * block, 0);
    std::memcpy(data.data(), "WBFS", 4);
    data[6] = 0x20; // 8192 sectors of 512 bytes
    data[8] = 9; data[9] = 20; data[12] = 1;
    data[512 + 257] = 2; data[512 + 259] = 3;
    data[3 * block - 2] = 11; data[3 * block - 1] = 22;
    data[3 * block] = 33; data[3 * block + 1] = 44;
    auto save = [&]() {
        std::ofstream out(path, std::ios::binary | std::ios::trunc);
        out.write(reinterpret_cast<const char*>(data.data()), data.size());
        if (!out) throw std::runtime_error("Fixture write failed");
    };
    auto check = [](bool value) { if (!value) throw std::runtime_error("WBFS unit test failed"); };
    save();
    {
        Wbfs reader(path);
        check(reader.read(block - 2, 4) == std::vector<uint8_t>({11,22,33,44}));
        check(reader.read(2 * block, 4) == std::vector<uint8_t>(4, 0));
        check(reader.read(Wbfs::capacity, 0).empty());
        bool rejected = false;
        try { reader.read(Wbfs::capacity - 1, 2); } catch (const std::runtime_error&) { rejected = true; }
        check(rejected);
    }
    auto rejected = [&]() {
        bool caught = false;
        try { Wbfs reader(path); } catch (const std::runtime_error&) { caught = true; }
        check(caught);
    };
    data[0] = 'X'; save(); rejected(); data[0] = 'W';
    data[9] = 63; save(); rejected(); data[9] = 20;
    data[512 + 257] = 9; save(); rejected(); data[512 + 257] = 2;
    data[13] = 1; save(); rejected(); data[13] = 0;
    data.pop_back(); save(); rejected();
}
