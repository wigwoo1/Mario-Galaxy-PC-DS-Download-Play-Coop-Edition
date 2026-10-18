#pragma once
#include <Game/System/ResourceInfo.hpp>
#include <Game/Util/JMapInfo.hpp>
#include <Game/Util/StringUtil.hpp>
#include <Native/HeapPlacement.hpp>
#include <JSystem/JKernel/JKRDisposer.hpp>
#include <cstdint>
#include <cstring>
#include <limits>
#include <stdexcept>
#include <vector>

namespace Native {
// NPCActorItem borrows strings from this table after its local JMapInfo dies.
// Keep the decoded image with the ResourceHolder's heap, and reuse it for
// subsequent lookups instead of converting or retaining another copy per NPC.
class NpcItemTableOwner final : public JKRDisposer {
    ResFileInfo& resource;
    void* original;
    JMapInfo table;
public:
    explicit NpcItemTableOwner(ResFileInfo& info) : resource(info), original(info.mResource) {
        const auto* bytes = static_cast<const std::uint8_t*>(original);
        table.attachNative(std::vector<std::uint8_t>(bytes, bytes + info._4));
    }
    const void* publish() {
        resource.mResource = const_cast<JMapData*>(table.mData);
        return resource.mResource;
    }
    ~NpcItemTableOwner() override {
        if (resource.mResource == table.mData) resource.mResource = original;
    }
};

inline const void* npcItemTable(ResFileInfo& resource, JKRHeap* heap) {
    // _8 and _4 are the original archive pointer and bounded resource extent,
    // filled by ResourceHolder::mount. Conversion never mutates those bytes.
    if (!heap || !resource.mResource || !resource._8 || resource._4 < 16 ||
        resource._4 > static_cast<u32>(std::numeric_limits<s32>::max()))
        throw std::invalid_argument("Invalid NPC item table archive extent or heap");
    if (resource.mResource != resource._8) return resource.mResource;
    return (new(heap, 0) NpcItemTableOwner(resource))->publish();
}

// Stationed heaps are trimmed after loading. Prepare NPC tables while the
// archive is registered so later item lookups need no extra heap allocation.
inline bool prepareNpcItemTable(const char* archiveName, ResFileInfo& resource, JKRHeap* heap) {
    if (!archiveName || !resource.mName || !MR::isEqualStringCase(archiveName,"NPCData")) return false;
    const auto length=std::strlen(resource.mName);
    if (length<9 || !MR::isEqualStringCase(resource.mName+length-9,"Item.bcsv")) return false;
    npcItemTable(resource,heap);
    return true;
}
}
