#pragma once
#include "wbfs.hpp"
#include "crypto.hpp"
#include <memory>
#include <set>
#include <string>

inline void assetCheck(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}
inline void extent(uint64_t offset, uint64_t size, uint64_t total) {
    assetCheck(offset <= total && size <= total - offset, "Asset extent outside data");
}
struct AssetFile { std::string path; uint64_t offset, size; };

inline void verifyH0(const std::vector<uint8_t>& hashes, const std::vector<uint8_t>& data) {
    assetCheck(hashes.size() == 0x400 && data.size() == 0x7c00, "Invalid decrypted cluster size");
    for (size_t i = 0; i < 31; ++i) {
        auto digest = sha1(data.data() + i * 0x400, 0x400);
        assetCheck(std::equal(digest.begin(), digest.end(), hashes.data() + i * 20), "Wii cluster H0 integrity failure");
    }
}

inline std::string tableString(const std::vector<uint8_t>& data, size_t start, size_t end) {
    assetCheck(start < end && end <= data.size(), "Invalid string offset");
    size_t p = start;
    while (p < end && data[p]) ++p;
    assetCheck(p < end, "Unterminated asset name");
    return std::string(reinterpret_cast<const char*>(data.data() + start), p - start);
}
inline void safeName(const std::string& name) {
    assetCheck(!name.empty() && name != "." && name != "..", "Invalid asset name");
    for (unsigned char c : name) assetCheck(c >= 32 && c != 127 && c != '/' && c != '\\' && c != ':', "Invalid asset path component");
}

inline std::vector<AssetFile> parseFst(const std::vector<uint8_t>& fst, uint64_t partitionSize, unsigned offsetShift = 2) {
    assetCheck(fst.size() >= 12 && fst[0] == 1 && be32(fst.data() + 4) == 0, "Invalid FST root");
    uint32_t count = be32(fst.data() + 8);
    assetCheck(count > 0 && count <= fst.size() / 12, "Invalid FST entry count");
    size_t strings = size_t(count) * 12;
    struct Directory { uint32_t index, end; std::string path; };
    std::vector<Directory> stack{{0, count, ""}};
    std::vector<AssetFile> files;
    std::set<std::string> paths;
    for (uint32_t i = 1; i < count; ++i) {
        while (!stack.empty() && i >= stack.back().end) stack.pop_back();
        assetCheck(!stack.empty(), "Invalid FST nesting");
        const uint8_t* entry = fst.data() + i * 12;
        std::string name = tableString(fst, strings + (be32(entry) & 0xffffff), fst.size());
        safeName(name);
        std::string path = stack.back().path + "/" + name;
        assetCheck(paths.insert(path).second, "Duplicate FST path");
        if (entry[0] == 1) {
            uint32_t end = be32(entry + 8);
            assetCheck(be32(entry + 4) == stack.back().index && end > i && end <= stack.back().end && stack.size() < 128, "Invalid FST directory");
            stack.push_back({i, end, path});
        } else {
            assetCheck(entry[0] == 0, "Unknown FST entry type");
            uint64_t offset = uint64_t(be32(entry + 4)) << offsetShift;
            uint64_t size = be32(entry + 8);
            extent(offset, size, partitionSize);
            files.push_back({path, offset, size});
        }
    }
    return files;
}

class WiiAssets {
    Wbfs disc;
    std::unique_ptr<AesCbc> aes;
    uint64_t dataStart = 0, plainSize = 0, cached = UINT64_MAX;
    std::vector<uint8_t> cache;
    uint64_t verified = 0;
    void loadCluster(uint64_t index) {
        if (cached == index) return;
        auto raw = disc.read(dataStart + index * 0x8000, 0x8000);
        std::array<uint8_t, 16> zero{};
        auto hashes = aes->decrypt(raw.data(), 0x400, zero.data());
        auto data = aes->decrypt(raw.data() + 0x400, 0x7c00, raw.data() + 0x3d0);
        verifyH0(hashes, data);
        cache = std::move(data); cached = index; ++verified;
    }
public:
    std::vector<AssetFile> files;
    explicit WiiAssets(const std::filesystem::path& path) : disc(path) {
        auto header = disc.read(0, 0x100);
        assetCheck(be32(header.data() + 0x18) == 0x5d1c9ea3 && header[0x60] == 0 && header[0x61] == 0, "Requires retail encrypted Wii disc with hashes");
        auto groups = disc.read(0x40000, 32);
        uint64_t partition = 0;
        for (int g = 0; g < 4 && !partition; ++g) {
            uint32_t count = be32(groups.data() + g * 8);
            assetCheck(count <= 1024, "Invalid partition count");
            auto table = disc.read(uint64_t(be32(groups.data() + g * 8 + 4)) * 4, count * 8);
            for (uint32_t i = 0; i < count; ++i) if (be32(table.data() + i * 8 + 4) == 0) {
                partition = uint64_t(be32(table.data() + i * 8)) * 4; break;
            }
        }
        assetCheck(partition != 0, "No game partition");
        auto ticket = disc.read(partition, 0x2c0);
        assetCheck(be32(ticket.data()) == 0x10001 && ticket[0x1f1] == 0, "Unsupported ticket signature or common-key index");
        // Retail Wii interoperability constant, also documented in Dolphin IOSC.
        const uint8_t common[16] = {0xeb,0xe4,0x2a,0x22,0x5e,0x85,0x93,0xe4,0x48,0xd9,0xc5,0x45,0x73,0x81,0xaa,0xf7};
        std::array<uint8_t, 16> iv{};
        std::copy_n(ticket.data() + 0x1dc, 8, iv.begin());
        AesCbc commonCipher(common);
        auto titleKey = commonCipher.decrypt(ticket.data() + 0x1bf, 16, iv.data());
        aes = std::make_unique<AesCbc>(titleKey.data());
        uint64_t dataOffset = uint64_t(be32(ticket.data() + 0x2b8)) * 4;
        uint64_t rawSize = uint64_t(be32(ticket.data() + 0x2bc)) * 4;
        extent(partition, dataOffset, Wbfs::capacity);
        dataStart = partition + dataOffset;
        extent(dataStart, rawSize, Wbfs::capacity);
        assetCheck(rawSize && rawSize % 0x8000 == 0, "Invalid partition size");
        plainSize = rawSize / 0x8000 * 0x7c00;
        auto boot = read(0, 0x440);
        assetCheck(std::equal(header.begin(), header.begin() + 6, boot.begin()), "Partition game ID mismatch");
        uint64_t fstOffset = uint64_t(be32(boot.data() + 0x424)) * 4;
        uint64_t fstSize = uint64_t(be32(boot.data() + 0x428)) * 4;
        assetCheck(fstSize <= 16 * 1024 * 1024, "Oversized FST");
        files = parseFst(read(fstOffset, static_cast<size_t>(fstSize)), plainSize);
    }
    std::vector<uint8_t> read(uint64_t offset, size_t size) {
        extent(offset, size, plainSize);
        assetCheck(size <= 256 * 1024 * 1024, "Asset read exceeds 256 MiB limit");
        std::vector<uint8_t> out(size);
        for (size_t done = 0; done < size;) {
            uint64_t position = offset + done;
            loadCluster(position / 0x7c00);
            size_t within = static_cast<size_t>(position % 0x7c00);
            size_t chunk = std::min(size - done, 0x7c00 - within);
            std::copy_n(cache.data() + within, chunk, out.data() + done);
            done += chunk;
        }
        return out;
    }
    std::vector<uint8_t> readFile(const std::string& path) {
        for (const auto& file : files) if (file.path == path) return read(file.offset, static_cast<size_t>(file.size));
        throw std::runtime_error("Disc file not found: " + path);
    }
    uint64_t verifiedClusters() const { return verified; }
};

inline std::vector<uint8_t> decodeYaz0(const std::vector<uint8_t>& src) {
    assetCheck(src.size() >= 16 && !std::memcmp(src.data(), "Yaz0", 4), "Invalid Yaz0 header");
    size_t size = be32(src.data() + 4);
    assetCheck(size <= 256 * 1024 * 1024, "Yaz0 output exceeds 256 MiB limit");
    std::vector<uint8_t> result;
    result.reserve(size);
    size_t pos = 16;
    auto byte = [&]() { assetCheck(pos < src.size(), "Truncated Yaz0 stream"); return src[pos++]; };
    while (result.size() < size) {
        uint8_t flags = byte();
        for (int mask = 128; mask && result.size() < size; mask >>= 1) {
            if (flags & mask) result.push_back(byte());
            else {
                uint8_t a = byte(), b = byte();
                size_t distance = ((a & 15) << 8 | b) + 1;
                size_t count = a >> 4;
                count = count ? count + 2 : size_t(byte()) + 18;
                assetCheck(distance <= result.size() && count <= size - result.size(), "Invalid Yaz0 back-reference");
                for (size_t i = 0; i < count; ++i) result.push_back(result[result.size() - distance]);
            }
        }
    }
    return result;
}

inline std::vector<uint8_t> decodeYay0(const std::vector<uint8_t>& src) {
    assetCheck(src.size() >= 16 && !std::memcmp(src.data(), "Yay0", 4), "Invalid Yay0 header");
    size_t size=be32(src.data()+4), links=be32(src.data()+8), bytes=be32(src.data()+12);
    assetCheck(size<=256*1024*1024, "Yay0 output exceeds 256 MiB limit");
    assetCheck(links>=16 && links<=bytes && bytes<=src.size(), "Invalid Yay0 table offsets");
    size_t maskPos=16, maskEnd=links, linkEnd=bytes; uint32_t bits=0; int remaining=0;
    std::vector<uint8_t> out; out.reserve(size);
    auto byte=[&](){assetCheck(bytes<src.size(), "Truncated Yay0 data");return src[bytes++];};
    while(out.size()<size){
        if(!remaining){assetCheck(maskPos<=maskEnd && maskEnd-maskPos>=4, "Truncated Yay0 masks");bits=be32(src.data()+maskPos);maskPos+=4;remaining=32;}
        if(bits&0x80000000u)out.push_back(byte());
        else {
            assetCheck(links<=linkEnd && linkEnd-links>=2, "Truncated Yay0 links");
            uint16_t link=uint16_t(src[links])<<8|src[links+1];links+=2;
            size_t distance=(link&4095)+1, count=link>>12;
            count=count?count+2:size_t(byte())+18;
            assetCheck(distance<=out.size() && count<=size-out.size(), "Invalid Yay0 back-reference");
            for(size_t i=0;i<count;++i)out.push_back(out[out.size()-distance]);
        }
        bits<<=1;--remaining;
    }
    return out;
}
class Rarc {
    std::vector<uint8_t> data;
public:
    std::vector<AssetFile> files;
    explicit Rarc(std::vector<uint8_t> bytes) : data(std::move(bytes)) {
        if (data.size() >= 4 && !std::memcmp(data.data(), "Yaz0", 4)) data = decodeYaz0(data);
        if (data.size() >= 4 && !std::memcmp(data.data(), "Yay0", 4)) data = decodeYay0(data);
        if (data.size() >= 0x20 && be32(data.data()) == 0x55aa382d) {
            uint32_t root = be32(data.data() + 4), headerSize = be32(data.data() + 8), payload = be32(data.data() + 12);
            assetCheck(root >= 0x20, "Invalid U8 root");
            extent(root, headerSize, data.size());
            assetCheck(payload >= uint64_t(root) + headerSize && payload <= data.size(), "Invalid U8 payload");
            std::vector<uint8_t> fst(data.begin() + root, data.begin() + root + headerSize);
            files = parseFst(fst, data.size(), 0);
            for (const auto& file : files) assetCheck(file.offset >= payload, "U8 file overlaps metadata");
            return;
        }
        assetCheck(data.size() >= 0x40 && !std::memcmp(data.data(), "RARC", 4), "Not a RARC archive");
        assetCheck(be32(data.data() + 4) == data.size() && be32(data.data() + 8) == 0x20, "Invalid RARC header");
        auto word = [&](size_t p) { extent(p, 4, data.size()); return be32(data.data() + p); };
        uint64_t payload = 0x20ull + word(0xc), payloadSize = word(0x10);
        uint32_t nodeCount = word(0x20), entryCount = word(0x28), stringsSize = word(0x30);
        uint64_t nodes = 0x20ull + word(0x24), entries = 0x20ull + word(0x2c), strings = 0x20ull + word(0x34);
        extent(payload, payloadSize, data.size()); extent(nodes, uint64_t(nodeCount) * 16, data.size());
        extent(entries, uint64_t(entryCount) * 20, data.size()); extent(strings, stringsSize, data.size());
        assetCheck(nodeCount > 0 && nodeCount <= 100000 && entryCount <= 1000000, "Invalid RARC table counts");
        struct Node { uint32_t index; std::string path; unsigned depth; };
        std::vector<Node> pending{{0, "", 0}};
        std::set<uint32_t> visited;
        std::set<std::string> paths;
        while (!pending.empty()) {
            Node node = pending.back(); pending.pop_back();
            assetCheck(node.index < nodeCount && node.depth < 128 && visited.insert(node.index).second, "RARC directory cycle or invalid node");
            size_t n = static_cast<size_t>(nodes + node.index * 16);
            uint32_t first = word(n + 12);
            uint32_t count = uint32_t(data[n+10]) << 8 | data[n+11];
            extent(first, count, entryCount);
            for (uint32_t i = first; i < first + count; ++i) {
                size_t e = static_cast<size_t>(entries + uint64_t(i) * 20);
                uint32_t flagsName = word(e + 4);
                std::string name = tableString(data, static_cast<size_t>(strings + (flagsName & 0xffffff)), static_cast<size_t>(strings + stringsSize));
                if (name == "." || name == "..") { assetCheck(flagsName & 0x02000000, "Invalid RARC dot entry"); continue; }
                safeName(name);
                std::string path = node.path + "/" + name;
                assetCheck(paths.insert(path).second, "Duplicate RARC path");
                if (flagsName & 0x02000000) pending.push_back({word(e + 8), path, node.depth + 1});
                else {
                    assetCheck(flagsName & 0x01000000, "Unsupported RARC entry");
                    uint64_t offset = word(e + 8), size = word(e + 12);
                    extent(offset, size, payloadSize);
                    files.push_back({path, payload + offset, size});
                }
            }
        }
    }
    std::vector<uint8_t> readFile(const std::string& path) const {
        for (const auto& file : files) if (file.path == path) {
            return {data.begin() + static_cast<size_t>(file.offset), data.begin() + static_cast<size_t>(file.offset + file.size)};
        }
        throw std::runtime_error("Archive resource not found: " + path);
    }
};


