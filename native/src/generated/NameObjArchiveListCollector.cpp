#include <cstring>
#include <stdexcept>
#include "Game/NameObj/NameObjArchiveListCollector.hpp"
#include "Game/Util/StringUtil.hpp"

NameObjArchiveListCollector::NameObjArchiveListCollector() : mArchiveNum() {
}

void NameObjArchiveListCollector::addArchive(const char* pName) {
    if (!pName || std::strlen(pName) >= sizeof(mArchive[0])) throw std::invalid_argument("Invalid native object archive name");
    if (mArchiveNum >= ARRAY_SIZE(mArchive)) throw std::length_error("Object archive collector capacity exceeded");
    MR::copyString(mArchive[mArchiveNum], pName, ARRAY_SIZE(*mArchive));

    mArchiveNum++;
}

const char* NameObjArchiveListCollector::getArchive(s32 index) const {
    if (index < 0 || index >= mArchiveNum) throw std::out_of_range("Object archive index");
    return mArchive[index];
}

