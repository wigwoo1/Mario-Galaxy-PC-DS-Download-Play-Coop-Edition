#include "Game/Util/HashUtil.hpp"
#include <cctype>
// Original hash recurrence; the console locale table is replaced by host tolower.
namespace MR {u32 getHashCode(const char* pStr) {u32 hash;for(hash=0;*pStr!='\0';pStr++){hash=*pStr+hash*31;}return hash;}}
namespace MR {
u32 getHashCodeLower(const char* pStr) {
    u32 hash=0;
    for(;*pStr!='\0';++pStr){
        const int ch=*pStr;
        // The original locale helper preserves negative/out-of-table characters.
        hash=(ch<0?ch:std::tolower(ch))+hash*31;
    }
    return hash;
}
}
