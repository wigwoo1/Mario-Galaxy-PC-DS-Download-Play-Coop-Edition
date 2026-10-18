#pragma once

#include <Native/GpuResources.hpp>
#include <JSystem/JKernel/JKRArchive.hpp>
#include <JSystem/JUtility/JUTTexture.hpp>
#include <revolution/gx/GXGet.h>

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace Native {
namespace detail {
struct BtiStorage {
    std::vector<std::uint8_t> bytes;

    BtiStorage(const void* source, std::size_t size) {
        if (source == nullptr || size < sizeof(ResTIMG)) {
            throw std::invalid_argument("truncated BTI header");
        }
        const auto* first = static_cast<const std::uint8_t*>(source);
        bytes.assign(first, first + size);

        const auto read16 = [this](std::size_t offset) -> std::uint16_t {
            return (std::uint16_t(bytes[offset]) << 8) | bytes[offset + 1];
        };
        const auto read32 = [this](std::size_t offset) -> std::uint32_t {
            return (std::uint32_t(bytes[offset]) << 24) | (std::uint32_t(bytes[offset + 1]) << 16) |
                   (std::uint32_t(bytes[offset + 2]) << 8) | bytes[offset + 3];
        };
        const auto extent = [size](std::size_t offset, std::size_t count) {
            if (offset > size || count > size - offset) {
                throw std::invalid_argument("BTI resource extent is out of bounds");
            }
        };

        ResTIMG header{};
        std::memcpy(&header, bytes.data(), sizeof(header));
        header.mWidth = read16(2);
        header.mHeight = read16(4);
        header.mPaletteNum = read16(10);
        header.mPaletteDataOffset = read32(12);
        const std::uint16_t lodBias = read16(26);
        header.mLodBias = static_cast<std::int16_t>(lodBias);
        header.mImageDataOffset = read32(28);

        if (bytes[16] > 1 || bytes[17] > 1 || bytes[18] > 1 || header.mWidth == 0 || header.mHeight == 0 ||
            header.mWidth > 1024 || header.mHeight > 1024 || header.mImageNum == 0 || header.mWrapS > 2 || header.mWrapT > 2 ||
            header.mMinType > 5 || header.mMagType > 1 || header.mMaxAnisotropy > 2) {
            throw std::invalid_argument("invalid BTI header fields");
        }

        switch (header.mFormat) {
        case GX_TF_I4:
        case GX_TF_I8:
        case GX_TF_IA4:
        case GX_TF_IA8:
        case GX_TF_RGB565:
        case GX_TF_RGB5A3:
        case GX_TF_RGBA8:
        case GX_TF_C4:
        case GX_TF_C8:
        case GX_TF_C14X2:
        case GX_TF_CMPR:
            break;
        default:
            throw std::invalid_argument("unsupported BTI texture format");
        }

        const std::size_t imageOffset = header.mImageDataOffset == 0 ? sizeof(ResTIMG) : header.mImageDataOffset;
        if (imageOffset < sizeof(ResTIMG) || imageOffset % 32 != 0) {
            throw std::invalid_argument("invalid BTI image offset");
        }
        std::size_t imageSize = 0;
        u16 levelWidth = header.mWidth;
        u16 levelHeight = header.mHeight;
        for (u8 level = 0; level < header.mImageNum; ++level) {
            imageSize += GXGetTexBufferSize(levelWidth, levelHeight, header.mFormat, GX_FALSE, 1);
            levelWidth = levelWidth > 1 ? levelWidth / 2 : 1;
            levelHeight = levelHeight > 1 ? levelHeight / 2 : 1;
        }
        extent(imageOffset, imageSize);

        if (header.mPaletteNum != 0) {
            if (header.mPaletteFormat > 2 || header.mPaletteDataOffset < sizeof(ResTIMG) || header.mPaletteDataOffset % 32 != 0) {
                throw std::invalid_argument("invalid BTI palette metadata");
            }
            extent(header.mPaletteDataOffset, std::size_t(header.mPaletteNum) * 2);
        }

        std::memcpy(bytes.data(), &header, sizeof(header));
    }

    const ResTIMG* texture() const {
        return reinterpret_cast<const ResTIMG*>(bytes.data());
    }
};

struct BtiRegistry {
    std::mutex mutex;
    std::unordered_map<const void*, std::shared_ptr<BtiStorage>> resources;
};

inline BtiRegistry& btiRegistry() {
    static BtiRegistry registry;
    return registry;
}
}  // namespace detail

// ResourceHolder may already provide a heap-owned normalized image. Keep this
// marker registry separate from the legacy raw-source cache and its layout.
struct NativeBtiHeaders {std::mutex mutex;std::unordered_set<const void*> headers;};
inline NativeBtiHeaders& nativeBtiHeaders(){static NativeBtiHeaders value;return value;}
inline void markNativeBti(const void* header,bool present){auto& known=nativeBtiHeaders();std::lock_guard<std::mutex> lock(known.mutex);if(present)known.headers.insert(header);else known.headers.erase(header);}
inline bool isNativeBti(const void* header){auto& known=nativeBtiHeaders();std::lock_guard<std::mutex> lock(known.mutex);return known.headers.count(header)!=0;}

// Archive BTI headers are big-endian, while JUTTexture consumes native fields.
// Keep a normalized copy alive and GPU-addressable for every returned pointer.
inline const ResTIMG* loadNativeBti(JKRArchive& archive, const void* source) {
    if (source == nullptr) {
        return nullptr;
    }
    if(isNativeBti(source))return static_cast<const ResTIMG*>(source);

    auto& registry = detail::btiRegistry();
    std::lock_guard<std::mutex> lock(registry.mutex);
    const auto existing = registry.resources.find(source);
    if (existing != registry.resources.end()) {
        return existing->second->texture();
    }

    const s32 resourceSize = archive.getResSize(source);
    if (resourceSize <= 0) {
        throw std::invalid_argument("BTI resource is not owned by its archive or is empty");
    }
    auto storage = std::make_shared<detail::BtiStorage>(source, static_cast<std::size_t>(resourceSize));
    gpuResources().addBorrowed(storage->bytes.data(), storage->bytes.size(), storage);
    const auto* result = storage->texture();
    registry.resources.emplace(source, std::move(storage));
    return result;
}
}  // namespace Native
