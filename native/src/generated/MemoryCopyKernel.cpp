#include <revolution/types.h>
#include <cstdint>
#include <cstring>
namespace MR {static u32 loadWord(const void* source){u32 word;std::memcpy(&word,source,4);return word;}static void storeWord(void* destination,u32 word){std::memcpy(destination,&word,4);}
    void copyMemory(void* pDst, const void* pSrc, u32 size) {
        u8* pDstBytes = static_cast< u8* >(pDst);
        const u8* pSrcBytes = static_cast< const u8* >(pSrc);
        u32 srcAlignment = reinterpret_cast< uintptr_t >(pSrc) & 3;
        u32 dstAlignment = reinterpret_cast< uintptr_t >(pDst) & 3;

        if (srcAlignment == 0 && dstAlignment == 0 && (size & 0xF) == 0) {
            const u32* pSrcWords = reinterpret_cast< const u32* >(pSrcBytes);
            u32* pDstWords = reinterpret_cast< u32* >(pDstBytes);

            for (u32 count = size / 16; count != 0; count--) {
                u32 word3, word2, word1, word0;
                word0 = loadWord(pSrcWords++);
                word1 = loadWord(pSrcWords++);
                word2 = loadWord(pSrcWords++);
                word3 = loadWord(pSrcWords++);
                storeWord(pDstWords++,word0);
                storeWord(pDstWords++,word1);
                storeWord(pDstWords++,word2);
                storeWord(pDstWords++,word3);
            }
        } else if (srcAlignment == dstAlignment && size >= 16) {
            if (srcAlignment != 0) {
                for (u8 count = 4 - srcAlignment; count != 0; count--) {
                    *pDstBytes++ = *pSrcBytes++;
                    size--;
                }
            }

            while (size >= 4) {
                std::memcpy(pDstBytes,pSrcBytes,4);
                size -= 4;
                pSrcBytes += 4;
                pDstBytes += 4;
            }

            if (size != 0) {
                for (; size != 0; size--) {
                    *pDstBytes++ = *pSrcBytes++;
                }
            }
        } else {
            for (; size != 0; size--) {
                *pDstBytes++ = *pSrcBytes++;
            }
        }
    }
}

