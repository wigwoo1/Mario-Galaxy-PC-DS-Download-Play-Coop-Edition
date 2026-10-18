#include <cstring>
// C ABI-independent original string implementations; diagnostic inline header is intentionally excluded.
namespace MR {
    const char* getBasename(const char* pPath) {
        const char* pBasename = strrchr(pPath, '/');

        if (pBasename == nullptr) {
            pBasename = pPath;
        } else {
            pBasename++;
        }

        return pBasename;
    }

    bool isEqualString(const char* pStr1, const char* pStr2) {
        return strcmp(pStr1, pStr2) == 0;
    }


}
#include <cwchar>
namespace MR {
    int strcasecmp(const char* pStr1, const char* pStr2) {
        return ::_stricmp(pStr1, pStr2);
    }
    bool isEqualStringCase(const char* pStr1, const char* pStr2) {
        return strcasecmp(pStr1, pStr2) == 0;
    }
    void copyString(char* pDst, const char* pSrc, unsigned int num) {
        strncpy(pDst, pSrc, num);
    }
    void copyString(wchar_t* pDst, const wchar_t* pSrc, unsigned int num) {
        wcsncpy(pDst, pSrc, num);
    }
}
