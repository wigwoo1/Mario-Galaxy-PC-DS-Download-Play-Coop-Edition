#include <Native/RemixSequenceData.hpp>

#include <limits>
#include <new>

namespace Native {
namespace {

class Reader {
public:
    Reader(const std::uint8_t* bytes, std::size_t size) : mBytes(bytes), mSize(size) {}

    bool readU32(std::uint32_t& value) {
        if (!mBytes || remaining() < 4) return false;
        value = (std::uint32_t(mBytes[mPosition]) << 24) |
                (std::uint32_t(mBytes[mPosition + 1]) << 16) |
                (std::uint32_t(mBytes[mPosition + 2]) << 8) |
                std::uint32_t(mBytes[mPosition + 3]);
        mPosition += 4;
        return true;
    }

    bool skipWords(std::uint32_t count) {
        if (count > std::numeric_limits<std::size_t>::max() / 4) return false;
        const auto bytes = std::size_t(count) * 4;
        if (bytes > remaining()) return false;
        mPosition += bytes;
        return true;
    }

    std::size_t remaining() const { return mSize - mPosition; }

private:
    const std::uint8_t* mBytes;
    std::size_t mSize;
    std::size_t mPosition = 0;
};

bool groupPayloadFits(std::uint32_t tracks, std::uint32_t notes, std::size_t remaining) {
    constexpr std::size_t wordBytes = 4;
    constexpr std::size_t noteBytes = sizeof(RemixNote);
    const auto max = std::numeric_limits<std::size_t>::max();
    const std::size_t noteCount = notes;
    const std::size_t trackCount = tracks;
    if (noteCount > max / wordBytes || noteCount > max / noteBytes) return false;
    const std::size_t orderBytes = noteCount * wordBytes;
    const std::size_t oneTrackBytes = wordBytes + noteCount * noteBytes;
    if (trackCount && oneTrackBytes > (max - orderBytes) / trackCount) return false;
    return orderBytes + trackCount * oneTrackBytes <= remaining;
}

} // namespace

bool decodeRemixSequenceData(const void* data, std::size_t byteCount, RemixSequenceData& out) {
    out.groups.clear();
    if (!data || byteCount < 4) return false;

    Reader reader(static_cast<const std::uint8_t*>(data), byteCount);
    std::uint32_t groupCount = 0;
    if (!reader.readU32(groupCount) || !reader.skipWords(groupCount)) return false;

    try {
        RemixSequenceData decoded;
        decoded.groups.reserve(groupCount);
        for (std::uint32_t groupIndex = 0; groupIndex < groupCount; ++groupIndex) {
            std::uint32_t trackCount = 0;
            std::uint32_t noteCount = 0;
            if (!reader.readU32(trackCount) || !reader.readU32(noteCount) ||
                !groupPayloadFits(trackCount, noteCount, reader.remaining())) return false;

            RemixGroup group;
            group.noteOrder.resize(noteCount);
            for (auto& note : group.noteOrder) {
                if (!reader.readU32(note)) return false;
            }

            group.tracks.resize(trackCount);
            for (auto& track : group.tracks) {
                std::uint32_t index = 0;
                if (!reader.readU32(index)) return false;
                track.index = static_cast<std::int32_t>(index);
                track.notes.resize(noteCount);
                for (auto& note : track.notes) {
                    for (auto& field : note.fields) {
                        std::uint32_t value = 0;
                        if (!reader.readU32(value)) return false;
                        field = static_cast<std::int32_t>(value);
                    }
                }
            }
            decoded.groups.push_back(std::move(group));
        }
        out = std::move(decoded);
        return true;
    } catch (const std::bad_alloc&) {
        out.groups.clear();
        return false;
    }
}

} // namespace Native
