#pragma once
#include <cstring>
namespace MR {
    // Game identifiers use ASCII case folding; UTF-8 bytes outside ASCII remain unchanged.
    inline unsigned char nativeIdentifierLower(unsigned char c){return c>='A'&&c<='Z'?static_cast<unsigned char>(c+('a'-'A')):c;}
    inline int nativeIdentifierCaseCompare(const char* a,const char* b){
        while(true){const auto left=nativeIdentifierLower(static_cast<unsigned char>(*a));const auto right=nativeIdentifierLower(static_cast<unsigned char>(*b));if(left!=right)return int(left)-int(right);if(left==0)return 0;++a;++b;}
    }
    inline int _stricmp(const char* a,const char* b){return nativeIdentifierCaseCompare(a,b);}
#ifndef strcasecmp
    inline int strcasecmp(const char* a,const char* b){return nativeIdentifierCaseCompare(a,b);}
#endif
    inline bool isEqualString(const char* a,const char* b){return std::strcmp(a,b)==0;}
    inline bool isEqualStringCase(const char* a,const char* b){return nativeIdentifierCaseCompare(a,b)==0;}
}
