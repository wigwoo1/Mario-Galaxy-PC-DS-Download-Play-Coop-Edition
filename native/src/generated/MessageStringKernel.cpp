#include "Game/Util/StringUtil.hpp"
namespace MR {
struct Tag {u16 marker;u8 mDataSize;u8 _3;wchar_t mBuffer[1];};
    bool isMessageEditorNextTag(const wchar_t* pStr) {
        const Tag* pTag = reinterpret_cast< const Tag* >(pStr);
        u8 v1 = pTag->_3;
        const auto* bytes = reinterpret_cast<const u8*>(pTag); u16 v2 = (u16(bytes[4]) << 8) | bytes[5];

        return v1 == 1 && v2 == 1;
    }

    int getStringLengthWithMessageTag(const wchar_t* pMessage) {
        int length = 0;

        while (*pMessage != '\0') {
            if (*pMessage == 0x1A) {
                if (isMessageEditorNextTag(pMessage)) {
                    break;
                }

                u16 dataSize = reinterpret_cast< const Tag* >(pMessage)->mDataSize;

                pMessage += dataSize / sizeof(u16);
                length += dataSize / sizeof(u16);
                pMessage--;
                length--;
            }

            pMessage++;
            length++;
        }

        return length;
    }


}

