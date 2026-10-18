#include <Native/RemixSequenceData.hpp>

#include <cassert>
#include <cstdint>
#include <vector>

namespace {
void pushBe32(std::vector<std::uint8_t>& bytes, std::uint32_t value) {
    bytes.push_back(std::uint8_t(value >> 24));
    bytes.push_back(std::uint8_t(value >> 16));
    bytes.push_back(std::uint8_t(value >> 8));
    bytes.push_back(std::uint8_t(value));
}
}

int main() {
    // Retail layout: group count, one ignored group-offset word, then group data.
    std::vector<std::uint8_t> resource;
    pushBe32(resource, 1);
    pushBe32(resource, 8);
    pushBe32(resource, 1); // tracks
    pushBe32(resource, 2); // notes
    pushBe32(resource, 0xA1);
    pushBe32(resource, 0xA2); // group note order
    pushBe32(resource, 7); // track id
    for (auto value : {0x10u, 0x11u, 0x12u, 0x13u, 0x20u, 0x21u, 0x22u, 0x23u})
        pushBe32(resource, value);

    Native::RemixSequenceData decoded;
    assert(Native::decodeRemixSequenceData(resource.data(), resource.size(), decoded));
    assert(decoded.groups.size() == 1);
    assert(decoded.groups[0].noteOrder.size() == 2);
    assert(decoded.groups[0].noteOrder[0] == 0xA1 && decoded.groups[0].noteOrder[1] == 0xA2);
    assert(decoded.groups[0].tracks.size() == 1);
    assert(decoded.groups[0].tracks[0].index == 7);
    assert(decoded.groups[0].tracks[0].notes.size() == 2);
    assert(decoded.groups[0].tracks[0].notes[0].fields[0] == 0x10);
    assert(decoded.groups[0].tracks[0].notes[1].fields[3] == 0x23);

    // Multiple empty groups, including an empty track/note table.
    std::vector<std::uint8_t> emptyGroups;
    pushBe32(emptyGroups, 2);
    pushBe32(emptyGroups, 0);
    pushBe32(emptyGroups, 4);
    pushBe32(emptyGroups, 0); pushBe32(emptyGroups, 0);
    pushBe32(emptyGroups, 0); pushBe32(emptyGroups, 3);
    pushBe32(emptyGroups, 0); pushBe32(emptyGroups, 0); pushBe32(emptyGroups, 0);
    assert(Native::decodeRemixSequenceData(emptyGroups.data(), emptyGroups.size(), decoded));
    assert(decoded.groups.size() == 2 && decoded.groups[1].tracks.empty());

    // Reject truncated headers/tables/payloads and the byte-swapped giant count.
    assert(!Native::decodeRemixSequenceData(nullptr, 0, decoded));
    assert(!Native::decodeRemixSequenceData(resource.data(), 3, decoded));
    assert(!Native::decodeRemixSequenceData(resource.data(), resource.size() - 4, decoded));
    std::vector<std::uint8_t> swapped;
    pushBe32(swapped, 0x06000000);
    assert(!Native::decodeRemixSequenceData(swapped.data(), swapped.size(), decoded));
    assert(decoded.groups.empty());
    return 0;
}
