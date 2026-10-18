#pragma once
#include <cstddef>
namespace Native {
// These APIs consume a normalized 32-byte ResTIMG. Disk offsets remain32-bit;
// replacement image/palette pointers are retained separately at host width.
void* textureImage(const void* header);
void* texturePalette(const void* header);
void copyTextureHeader(void* destination,const void* source);
void forgetTexturePointers(const void* range,std::size_t size);
}
