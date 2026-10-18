#pragma once

#include "JSystem/JAudio2/JAISound.hpp"
#include "JSystem/JAudio2/JASGlobal.hpp"

inline u16 nativeSoundTableU16(const void* ptr) { const auto* b=static_cast<const u8*>(ptr); return (u16(b[0])<<8)|b[1]; }
inline u32 nativeSoundTableU32(const void* ptr) { const auto* b=static_cast<const u8*>(ptr); return (u32(b[0])<<24)|(u32(b[1])<<16)|(u32(b[2])<<8)|b[3]; }
struct JAUSoundTableItem {
    /* 0x00 */ u8 mPriority;
    /* 0x01 */ u8 mVolume;
    /* 0x02 */ u16 mResourceId;
    /* 0x04 */ u8 _4[0];
};

struct JAUSoundTableSe {
    /* 0x00 */ u8 mPriority;
    /* 0x01 */ u8 mVolume;
    /* 0x02 */ u16 mAudibleSw;
    /* 0x04 */ u16 mSoundSw;
};

struct JAUSoundTableBgm {
    /* 0x00 */ u8 mPriority;
    /* 0x01 */ u8 mVolume;
    /* 0x02 */ u16 mResourceId;
    /* 0x04 */ u16 mChordResId;
    /* 0x06 */ u16 mSoundSw;
};

struct JAUSoundTableStream {
    /* 0x00 */ u8 mPriority;
    /* 0x01 */ u8 mVolume;
    /* 0x02 */ u16 mChannelCtrl;
    /* 0x04 */ u32 mStreamFileNameOffset;
};

template < typename Root, typename Section, typename Group, typename Item >
struct JAUSoundTable_ {
    JAUSoundTable_() {
        mData = nullptr;
        mRoot = nullptr;
    }

    void reset() {
        mData = nullptr;
        mRoot = nullptr;
    }

    void init(const void* pData) {
        reset();
        if (!pData) return;
        mData = pData;
        if (nativeSoundTableU32(mData) != Root::magicNumber()) {
            mData = nullptr;
        } else {
            mRoot = (Root*)((u8*)mData + nativeSoundTableU32(static_cast<const u8*>(mData)+12));
        }
    }

    Section* getSection(int index) const {
        if (index < 0) {
            return nullptr;
        }
        if (!mRoot || (u32)index >= nativeSoundTableU32(&mRoot->mSectionNumber)) {
            return nullptr;
        }
        u32 offset = nativeSoundTableU32(reinterpret_cast<const u8*>(mRoot)+4+index*4);
        if (offset == 0) {
            return nullptr;
        }
        return (Section*)((u8*)mData + offset);
    }

    Group* getGroup(const Section* section, int index) const {
        if (index < 0) {
            return nullptr;
        }
        if (!mData || !section || (u32)index >= nativeSoundTableU32(&section->mNumGroups)) {
            return nullptr;
        }
        u32 offset = section->getGroupOffset(index);
        if (offset == 0) {
            return nullptr;
        }
        return (Group*)((u8*)mData + offset);
    }

    Item* getItem(const Group* group, int index) const {
        if (!mData || !group || index < 0) return nullptr;
        u32 offset = group->getItemOffset(index);
        if (offset == 0) {
            return nullptr;
        }
        return (Item*)((u8*)mData + offset);
    }

    /* 0x00 */ const void* mData;
    /* 0x04 */ Root* mRoot;
};

struct JAUSoundTableRoot {
    static inline u32 magicNumber() {
        return 'BST ';
    }
    /* 0x00 */ u32 mSectionNumber;
    /* 0x04 */ u32 mSectionOffsets[0];
};

struct JAUSoundTableSection {
    int getGroupOffset(int index) const {
        if (index < 0) {
            return 0;
        }
        if (u32(index) >= nativeSoundTableU32(&mNumGroups)) {
            return 0;
        }
        return nativeSoundTableU32(reinterpret_cast<const u8*>(mGroupOffsets)+index*4);
    }

    /* 0x00 */ u32 mNumGroups;
    /* 0x04 */ u32 mGroupOffsets[0];
};

struct JAUSoundTableGroup {
    u8 getTypeID(int index) const {
        if (index < 0) {
            return 0;
        }
        if (u32(index) >= nativeSoundTableU32(&mNumItems)) {
            return -1;
        }
        return ((u8*)mItemOffsets)[index * 4];
    }

    u32 getItemOffset(int index) const {
        if (index < 0) {
            return 0;
        }
        if (u32(index) >= nativeSoundTableU32(&mNumItems)) {
            return 0;
        }
        return nativeSoundTableU32(reinterpret_cast<const u8*>(mItemOffsets)+index*4) & 0xffffff;
    }

    /* 0x00 */ u32 mNumItems;
    /* 0x04 */ u32 mGlobalOffset;
    /* 0x08 */ u32 mItemOffsets[0];
};

struct JAUSoundTable : public JASGlobalInstance< JAUSoundTable > {
    JAUSoundTable(bool setInstance) : JASGlobalInstance< JAUSoundTable >(setInstance) {
    }
    ~JAUSoundTable() {
    }

    u32 nativeResourceSize=0;
    void init(void const*);
    u8 getTypeID(JAISoundID) const;
    JAUSoundTableItem* getData(JAISoundID) const;
    int getNumGroups_inSection(u8) const;
    int getNumItems_inGroup(u8, u8) const;

    JAUSoundTableItem* getItem(const JAUSoundTableGroup* group, int index) const {
        return (JAUSoundTableItem*)mTable.getItem(group, index);
    }

    const void* getResource() const {
        return mTable.mData;
    }
    bool isValid() const {
        return mTable.mData != nullptr;
    }

    /* 0x00 */ JAUSoundTable_< JAUSoundTableRoot, JAUSoundTableSection, JAUSoundTableGroup, void > mTable;
};

struct JAUSoundNameTableRoot {
    static inline u32 magicNumber() {
        return 'BSTN';
    }
    /* 0x00 */ u32 mSectionNumber;
    /* 0x04 */ u32 mSectionOffsets[0];
};

struct JAUSoundNameTableSection {
    int getGroupOffset(int index) const {
        if (index < 0) {
            return 0;
        }
        if (u32(index) >= nativeSoundTableU32(&mNumGroups)) {
            return 0;
        }
        return nativeSoundTableU32(reinterpret_cast<const u8*>(mGroupOffsets)+index*4);
    }

    /* 0x00 */ u32 mNumGroups;
    /* 0x04 */ u32 mGlobalOffset;
    /* 0x08 */ u32 mGroupOffsets[0];
};

struct JAUSoundNameTableGroup {
    u32 getItemOffset(int index) const {
        if (index < 0) {
            return 0;
        }
        if (u32(index) >= nativeSoundTableU32(&mNumItems)) {
            return 0;
        }
        return nativeSoundTableU32(reinterpret_cast<const u8*>(mItemOffsets)+index*4);
    }

    /* 0x00 */ u32 mNumItems;
    /* 0x04 */ u32 mCategoryNameOffset;
    /* 0x08 */ u32 mItemOffsets[0];
};

struct JAUSoundNameTable : public JASGlobalInstance< JAUSoundNameTable > {
    JAUSoundNameTable(bool setInstance) : JASGlobalInstance< JAUSoundNameTable >(setInstance) {
    }
    ~JAUSoundNameTable() {
    }
    int getNumGroups_inSection(u8) const;
    int getNumItems_inGroup(u8, u8) const;
    void init(void const*);
    const char* getName(JAISoundID) const;
    const char* getGroupName(JAISoundID) const;

    const char* getItem(const JAUSoundNameTableGroup* group, int index) const {
        return (const char*)mTable.getItem(group, index);
    }

    /* 0x00 */ JAUSoundTable_< JAUSoundNameTableRoot, JAUSoundNameTableSection, JAUSoundNameTableGroup, char > mTable;
};

