#include "Game/Util/StringUtil.hpp"
#include <cstdio>
#include <cstring>
#include <cctype>
#include <revolution/types.h>
namespace MR {
    void scan16(const char* pSrc, const char* pSubStr, u16* pDst) {
        int temp;

        if (strstr(pSrc, pSubStr) == nullptr) {
            return;
        }

        sscanf(pSrc, "\t%d", &temp);

        *pDst = temp;
    }
    void scans16x4(const char* pSrc, const char* pSubStr, s16* pDst) {
        int temp[4];

        if (strstr(pSrc, pSubStr) == nullptr) {
            return;
        }

        sscanf(pSrc, "\t{%d,%d,%d,%d}", &temp[0], &temp[1], &temp[2], &temp[3]);

        for (int i = 0; i < ARRAY_SIZE(temp); i++) {
            pDst[i] = temp[i];
        }
    }
    void scan8(const char* pSrc, const char* pSubStr, u8* pDst) {
        int temp;

        if (strstr(pSrc, pSubStr) == nullptr) {
            return;
        }

        sscanf(pSrc, "\t%d", &temp);

        *pDst = temp;
    }
    void scan32(const char* pSrc, const char* pSubStr, s32* pDst) {
        if (strstr(pSrc, pSubStr) == nullptr) {
            return;
        }

        sscanf(pSrc, "\t%d", pDst);
    }
    void scanf32(const char* pSrc, const char* pSubStr, f32* pDst) {
        if (strstr(pSrc, pSubStr) == nullptr) {
            return;
        }

        sscanf(pSrc, "\t%ff", pDst);
    }
    void scanu8x4(const char* pSrc, const char* pSubStr, u8* pDst) {
        int temp[4];

        if (strstr(pSrc, pSubStr) == nullptr) {
            return;
        }

        sscanf(pSrc, "\t{%d,%d,%d,%d}", &temp[0], &temp[1], &temp[2], &temp[3]);

        for (int i = 0; i < ARRAY_SIZE(temp); i++) {
            pDst[i] = temp[i];
        }
    }
    void scanf32x4(const char* pSrc, const char* pSubStr, f32* pDst) {
        if (strstr(pSrc, pSubStr) == nullptr) {
            return;
        }

        sscanf(pSrc, "\t{%ff,%ff,%ff,%ff}", &pDst[0], &pDst[1], &pDst[2], &pDst[3]);
    }
    bool isEqualSubString(const char* pStr, const char* pSubStr) {
        return strstr(pStr, pSubStr) != nullptr;
    }
    bool hasStringSpace(const char* pStr) {
        return strchr(pStr, ' ') != nullptr;
    }
    bool isDigitStringTail(const char* pStr, int digitNum) {
        for (int i = 1; i <= digitNum; i++) {
            int ch = pStr[strlen(pStr) - i];

            if (isdigit(ch) == 0) {
                return false;
            }
        }

        return true;
    }
    bool isNullOrEmptyString(const char* pStr) {
        return pStr == nullptr || MR::isEqualString(pStr, "");
    }
}

