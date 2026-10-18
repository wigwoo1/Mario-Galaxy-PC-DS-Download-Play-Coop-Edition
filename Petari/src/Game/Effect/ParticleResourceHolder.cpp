#include "Game/Effect/ParticleResourceHolder.hpp"
#include "Game/Util/FileUtil.hpp"
#include "Game/Util/JMapInfo.hpp"
#include "Game/Util/MemoryUtil.hpp"
#include "Game/Util/StringUtil.hpp"
#include <JSystem/JKernel/JKRMemArchive.hpp>
#include <JSystem/JParticle/JPAResourceManager.hpp>
#include <JSystem/JKernel/JKRHeap.hpp>
#include <Native/JpaArchive.hpp>
#include <Native/GpuResources.hpp>
#include <Native/HeapPlacement.hpp>
#include <JSystem/JKernel/JKRDisposer.hpp>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <vector>

namespace {
class ParticleImageMapping : public JKRDisposer {
    const void* mImage;
public:
    ParticleImageMapping(const void* image, std::size_t size) : mImage(image) {
        Native::gpuResources().add(image, size);
    }
    ~ParticleImageMapping() override {
        Native::gpuResources().remove(mImage);
    }
};
void attachNativeJMapResource(JMapInfo& info, JKRArchive* archive, const char* name) {
    if (archive == nullptr) {
        throw std::runtime_error("ParticleResourceHolder archive is null");
    }
    const void* source = archive->getResource(name);
    if (source == nullptr) {
        throw std::runtime_error("ParticleResourceHolder JMap resource is missing");
    }
    const s32 size = archive->getResSize(source);
    if (size <= 0) {
        throw std::runtime_error("ParticleResourceHolder JMap resource has no archive extent");
    }
    const auto* first = static_cast<const std::uint8_t*>(source);
    const std::vector<std::uint8_t> bytes(first, first + size);
    if (!info.attachNative(bytes)) {
        throw std::runtime_error("ParticleResourceHolder could not attach native JMap data");
    }
}
}  // namespace

ParticleResourceHolder::ParticleResourceHolder(const char* pArchiveName)
    : mResourceMgr(), mAutoEffectList(new JMapInfo), mParticleNames(new JMapInfo), mNumEffectNums() {
    JKRMemArchive* pArchive = MR::mountArchive(pArchiveName, nullptr);
    const void* particleSource = pArchive->getResource("Particles.jpc");
    const s32 particleSize = particleSource ? pArchive->getResSize(particleSource) : 0;
    if (particleSize <= 0) {
        throw std::runtime_error("ParticleResourceHolder particle archive has no resource extent");
    }
    const auto nativeData = NativeJpa::normalize(particleSource, static_cast<std::size_t>(particleSize));
    JKRHeap* particleHeap = MR::getCurrentHeap();
    // JPA blocks retain pointers into this image. Like the resource/texture
    // arrays, it belongs to the particle heap and lives until heap teardown.
    void* particleImage = JKRHeap::alloc(static_cast<u32>(nativeData.size()), 32, particleHeap);
    if (!particleImage) throw std::bad_alloc();
    std::memcpy(particleImage, nativeData.data(), nativeData.size());
    ParticleImageMapping* imageMapping = nullptr;
    try {
        imageMapping = new (particleHeap, 0) ParticleImageMapping(particleImage, nativeData.size());
        mResourceMgr = new JPAResourceManager(particleImage, particleHeap);
    } catch (...) {
        if (imageMapping) {
            imageMapping->~ParticleImageMapping();
            JKRHeap::free(imageMapping, particleHeap);
        }
        JKRHeap::free(particleImage, particleHeap);
        throw;
    }
    attachNativeJMapResource(*mParticleNames, pArchive, "ParticleNames.bcsv");
    attachNativeJMapResource(*mAutoEffectList, pArchive, "AutoEffectList.bcsv");
    countAutoEffectNum();
}

u16 ParticleResourceHolder::getUserIndex(const char* pName) const {
    const JMapInfo* pInfo = mParticleNames;
    int middle;
    int last;
    int first = 0;
    last = pInfo->getNumEntries() - 1;

    const char* pEntryName;

    while (first < last) {
        middle = (first + last) / 2;
        const char* pEntryName = "";

        if (!pInfo->getValue(middle, "name", &pEntryName)) {
            return 0xFFFF;
        }

        const int comparison = strcmp(pEntryName, pName);

        if (comparison == 0) {
            return middle;
        }

        if (comparison < 0) {
            first = middle + 1;
        }

        if (comparison >= 0) {
            last = middle;
        }
    }

    pEntryName = "";

    if (!pInfo->getValue(first, "name", &pEntryName)) {
        return 0xFFFF;
    }

    if (strcmp(pEntryName, pName) == 0) {
        return first;
    }

    return 0xFFFF;
}

void ParticleResourceHolder::countAutoEffectNum() {
    for (JMapInfoIter iter = mAutoEffectList->begin(); iter != mAutoEffectList->end(); iter.mIndex++) {
        const char* pGroupName = nullptr;
        iter.getValue("GroupName", &pGroupName);

        if (pGroupName != nullptr) {
            bool found = false;

            for (EffectNum** pParticle = mEffectNums; pParticle != mEffectNums + mNumEffectNums; pParticle++) {
                if (MR::isEqualStringCase((*pParticle)->mGroupName, pGroupName)) {
                    (*pParticle)->mCount++;
                    found = true;
                    break;
                }
            }

            if (!found) {
                EffectNum* pParticle = new EffectNum(pGroupName);
                const int index = mNumEffectNums++;
                mEffectNums[index] = pParticle;
            }
        }
    }
}

void ParticleResourceHolder::swapTexture(const ResTIMG* pImage, const char* pName) {
    mResourceMgr->swapTexture(pImage, pName);
}

bool ParticleResourceHolder::isExistInResource(const char* pName, u16* pIndex) const {
    const u16 index = getUserIndex(pName);

    if (index == 0xFFFF) {
        return false;
    }

    if (pIndex != nullptr) {
        *pIndex = index;
    }

    return true;
}

JMapInfo* ParticleResourceHolder::getAutoEffectListBinary() const {
    return mAutoEffectList;
}

int ParticleResourceHolder::getAutoEffectNum(const char* pGroupName) const {
    if (pGroupName == nullptr) {
        return 0;
    }

    for (EffectNum* const* pParticle = mEffectNums; pParticle != mEffectNums + mNumEffectNums; pParticle++) {
        if (MR::isEqualStringCase((*pParticle)->mGroupName, pGroupName)) {
            return (*pParticle)->mCount;
        }
    }

    return 0;
}
