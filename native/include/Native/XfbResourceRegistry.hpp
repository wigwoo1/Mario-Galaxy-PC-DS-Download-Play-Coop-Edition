#pragma once
#include <Native/GpuResources.hpp>
#include <array>
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <vector>

namespace Native {
// Registers externally owned JUTXfb buffers as writable guest GPU resources.
// The caller owns their storage and must call clear before destroying it.
class XfbResourceRegistry {
    struct Entry { void* buffer; std::size_t bytes; bool owned; };
    std::vector<Entry> entries;
public:
    void refresh(GpuResources& resources, const std::array<void*, 3>& buffers,
                 unsigned count, unsigned width, unsigned height) {
        if (count > buffers.size() || !width || !height || width > 1024 || height > 1024)
            throw std::invalid_argument("Invalid native XFB registry dimensions");
        const auto rowBytes = std::size_t(width) * 2;
        if (rowBytes > SIZE_MAX / height) throw std::overflow_error("Native XFB size overflow");
        const auto bytes = rowBytes * height;

        std::vector<Entry> next;
        next.reserve(count);
        for (unsigned i = 0; i < count; ++i) {
            auto* buffer = buffers[i];
            if (!buffer) continue;
            bool already=false;
            for (const auto& entry : next) {
                if (entry.buffer == buffer) {
                    if (entry.bytes != bytes) throw std::invalid_argument("Aliased XFB buffers have inconsistent dimensions");
                    already=true; break;
                }
            }
            if (already) continue;
            auto previous=std::find_if(entries.begin(),entries.end(),[&](const Entry& entry){return entry.buffer==buffer&&entry.bytes==bytes;});
            if(previous!=entries.end()){next.push_back(*previous);continue;}

            bool owned=false;
            try {
                const auto address=resources.address(buffer);
                (void)resources.resolve(address,bytes);
            } catch (const std::out_of_range&) {
                resources.addWritable(buffer,bytes);
                owned=true;
            } catch (const std::invalid_argument&) {
                resources.addWritable(buffer,bytes);
                owned=true;
            }
            next.push_back({buffer,bytes,owned});
        }

        for (const auto& old : entries) {
            const bool retained=std::any_of(next.begin(),next.end(),[&](const Entry& value){return value.buffer==old.buffer&&value.bytes==old.bytes;});
            if(old.owned&&!retained)resources.remove(old.buffer);
        }
        entries=std::move(next);
    }

    void clear(GpuResources& resources) {
        for (const auto& entry : entries) if (entry.owned) resources.remove(entry.buffer);
        entries.clear();
    }
    std::size_t registeredCount() const noexcept { return entries.size(); }
};
}
