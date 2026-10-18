#include <stdexcept>
#include <Native/TplResources.hpp>
#include "nw4r/lyt/common.h"
#include "nw4r/lyt/texMap.h"
#include <revolution/gx/GXEnum.h>
#include <revolution/gx/GXGet.h>
#include <revolution/gx/GXStruct.h>
#include <revolution/tpl.h>

namespace nw4r {
    namespace lyt {
        void TexMap::Get(_GXTexObj* pTexObj) const {
            if (detail::IsCITexelFormat(GetTexelFormat())) {
                u32 tlutName = GXGetTexObjTlut(pTexObj);
                GXInitTexObjCI(pTexObj, mImage, mWidth, mHeight, GXCITexFmt(GetTexelFormat()), GetWrapModeS(), GetWrapModeT(), IsMipMap(), tlutName);
            } else {
                GXInitTexObj(pTexObj, mImage, mWidth, mHeight, GetTexelFormat(), GetWrapModeS(), GetWrapModeT(), IsMipMap());
            }

            GXInitTexObjLOD(pTexObj, GetMinFilter(), GetMagFilter(), GetMinLOD(), GetMaxLOD(), GetLODBias(), IsBiasClampEnable(), IsEdgeLODEnable(),
                            GetAnisotropy());
        }

        void TexMap::Get(_GXTlutObj* pTlutObj) const {
            GXInitTlutObj(pTlutObj, GetPalette(), GetPaletteFormat(), GetPaletteEntryNum());
        }

        void TexMap::Set(const GXTexObj& texObj) {
            void* image;
            u16 width, height;
            GXTexFmt format;
            GXTexWrapMode wrapS, wrapT;
            GXBool mipmap;

            GXGetTexObjAll(&texObj, &image, &width, &height, &format, &wrapS, &wrapT, &mipmap);

            mImage = image;
            SetSize(width, height);
            mBits.textureFormat = format;
            SetWrapMode(wrapS, wrapT);
            SetMipMap(mipmap);

            GXTexFilter minFilter, magFilter;
            f32 minLOD, maxLOD, lodBias;
            GXBool biasCLampEnable, edgeLODEnable;
            GXAnisotropy aniso;
            GXGetTexObjLODAll(&texObj, &minFilter, &magFilter, &minLOD, &maxLOD, &lodBias, &biasCLampEnable, &edgeLODEnable, &aniso);

            SetFilter(minFilter, magFilter);
            SetLOD(minLOD, maxLOD);
            SetLODBias(lodBias);
            SetBiasClampEnable(biasCLampEnable);
            SetEdgeLODEnable(edgeLODEnable);
            mBits.anisotropy = aniso;
        }

        void TexMap::ReplaceImage(const TPLDescriptor* pTPLDesc) {
            const TPLHeader& header = *pTPLDesc->textureHeader;
            mImage = header.data;
            SetSize(header.width, header.height);
            SetTexelFormat(GXTexFmt(header.format));

            if (const TPLClutHeader* const pClut = pTPLDesc->CLUTHeader) {
                SetPalette(pClut->data);
                SetPaletteFormat(pClut->format);
                SetPaletteEntryNum(pClut->numEntries);
            } else {
                SetPalette(nullptr);
                SetPaletteFormat(GXTlutFmt(0));
                SetPaletteEntryNum(0);
            }
        }

        void TexMap::ReplaceImage(TPLPalette* p, u32 id) {
            if (p == nullptr) throw std::invalid_argument("null TPL resource");
            const u8* const bytes = reinterpret_cast<const u8*>(p);
            const std::size_t size = Native::tplResourceSize(p);
            if (size < 12) throw std::invalid_argument("unregistered or truncated TPL resource");

            const auto read16 = [bytes, size](std::size_t offset) -> u16 {
                if (offset > size || size - offset < 2) throw std::invalid_argument("truncated TPL field");
                return detail::ReadBigEndianU16(bytes + offset);
            };
            const auto read32 = [bytes, size](std::size_t offset) -> u32 {
                if (offset > size || size - offset < 4) throw std::invalid_argument("truncated TPL field");
                return detail::ReadBigEndianU32(bytes + offset);
            };
            const auto extent = [size](std::size_t offset, std::size_t length) {
                if (offset > size || length > size - offset) throw std::invalid_argument("TPL resource extent is out of bounds");
            };

            if (read32(0) != 0x0020AF30) throw std::invalid_argument("invalid TPL resource signature");
            const u32 descriptorCount = read32(4);
            const u32 descriptorTable = read32(8);
            if (descriptorCount == 0 || descriptorCount > 4096) throw std::invalid_argument("invalid TPL descriptor count");
            extent(descriptorTable, std::size_t(descriptorCount) * 8);
            id %= descriptorCount;

            const std::size_t descriptor = descriptorTable + std::size_t(id) * 8;
            const u32 headerOffset = read32(descriptor);
            const u32 paletteOffset = read32(descriptor + 4);
            extent(headerOffset, 36);

            const u16 height = read16(headerOffset);
            const u16 width = read16(headerOffset + 2);
            const u32 format = read32(headerOffset + 4);
            const u32 imageOffset = read32(headerOffset + 8);
            if (width == 0 || height == 0 || width > 8192 || height > 8192 ||
                std::size_t(width) * height > 16777216) {
                throw std::invalid_argument("invalid TPL texture dimensions");
            }

            std::size_t blockWidth = 4, blockHeight = 4, bytesPerBlock = 32;
            switch (format) {
            case 0: case 8: blockWidth = blockHeight = 8; break;
            case 1: case 2: case 9: blockWidth = 8; break;
            case 3: case 4: case 5: case 10: break;
            case 6: bytesPerBlock = 64; break;
            case 14: blockWidth = blockHeight = 8; break;
            default: throw std::invalid_argument("unsupported TPL texture format");
            }
            const std::size_t imageBytes = ((std::size_t(width) + blockWidth - 1) / blockWidth) *
                                           ((std::size_t(height) + blockHeight - 1) / blockHeight) * bytesPerBlock;
            extent(imageOffset, imageBytes);

            SetImage(const_cast<u8*>(bytes + imageOffset));
            SetSize(width, height);
            SetTexelFormat(GXTexFmt(format));

            if (paletteOffset != 0) {
                extent(paletteOffset, 12);
                const u16 entryCount = read16(paletteOffset);
                const u32 paletteFormat = read32(paletteOffset + 4);
                const u32 paletteDataOffset = read32(paletteOffset + 8);
                if (entryCount == 0 || entryCount > 16384 || paletteFormat > 2) {
                    throw std::invalid_argument("invalid TPL palette metadata");
                }
                extent(paletteDataOffset, std::size_t(entryCount) * 2);
                SetPalette(const_cast<u8*>(bytes + paletteDataOffset));
                SetPaletteFormat(GXTlutFmt(paletteFormat));
                SetPaletteEntryNum(entryCount);
            } else {
                SetPalette(nullptr);
                SetPaletteFormat(GXTlutFmt(0));
                SetPaletteEntryNum(0);
            }
        }
    };  // namespace lyt
};  // namespace nw4r
