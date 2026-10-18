#include "native_jmap_storage.hpp"
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

static std::vector<std::uint8_t> readFile(const char* path) {
    std::ifstream input(path, std::ios::binary);
    if (!input) throw std::runtime_error(std::string("Cannot read BCSV: ") + path);
    return {std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>()};
}

static void emit(std::ostream& out, const char* name, const std::vector<std::uint8_t>& bytes) {
    out << "extern const std::uint8_t " << name << "[] = {\n";
    for (std::size_t i = 0; i < bytes.size(); ++i) {
        if (i % 16 == 0) out << "    ";
        out << static_cast<unsigned>(bytes[i]);
        if (i + 1 != bytes.size()) out << ',';
        if (i % 16 == 15 || i + 1 == bytes.size()) out << '\n';
        else out << ' ';
    }
    out << "};\n";
}

int main(int argc, char** argv) {
    try {
        if (argc != 4) throw std::runtime_error("Expected StoryEvent.bcsv, GalaxyID.bcsv, and output C++ path");
        auto storyDisk = readFile(argv[1]);
        auto galaxyDisk = readFile(argv[2]);
        NativeJMapStorage storyHost(storyDisk);
        NativeJMapStorage galaxyHost(galaxyDisk);
        std::ofstream output(argv[3], std::ios::binary | std::ios::trunc);
        if (!output) throw std::runtime_error("Cannot create generated retail-table provider");
        output << "// Generated from the local Petari BCSV inputs; normalized for host-endian JMapInfo.\n"
                  "#include <cstdint>\n"
                  "// These symbols are byte blobs in the original DOL. JMapData's flexible-array declaration is a view.\n";
        std::vector<std::uint8_t> story(static_cast<const std::uint8_t*>(storyHost.data()), static_cast<const std::uint8_t*>(storyHost.data()) + storyDisk.size());
        std::vector<std::uint8_t> galaxy(static_cast<const std::uint8_t*>(galaxyHost.data()), static_cast<const std::uint8_t*>(galaxyHost.data()) + galaxyDisk.size());
        emit(output, "StoryEventBCSV", story);
        emit(output, "GalaxyIDBCSV", galaxy);
        if (!output) throw std::runtime_error("Failed writing generated retail-table provider");
        std::cout << "Generated host-endian StoryEventBCSV (" << story.size() << " bytes) and GalaxyIDBCSV ("
                  << galaxy.size() << " bytes).\n";
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}