#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

namespace Native {

struct RemixNote {
    std::int32_t fields[4]{};
};

struct RemixTrack {
    std::int32_t index = 0;
    std::vector<RemixNote> notes;
};

struct RemixGroup {
    std::vector<std::uint32_t> noteOrder;
    std::vector<RemixTrack> tracks;
};

struct RemixSequenceData {
    std::vector<RemixGroup> groups;
};

// Decodes the retail big-endian Remix sequence table without retaining pointers
// into the archive's packed resource. False means the entire bounded record is
// malformed or truncated; out is left empty in that case.
bool decodeRemixSequenceData(const void* bytes, std::size_t byteCount, RemixSequenceData& out);

} // namespace Native
