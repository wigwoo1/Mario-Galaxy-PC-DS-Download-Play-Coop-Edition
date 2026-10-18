#include <Game/System/DrawSyncManager.hpp>
u16 DrawSyncManager::setCallback(u32 index, u16 count, DrawSyncCallback* pCallback) {
    if (index < 3) {
        u16 start = mLowTokenCount + 1;
        mTokenRanges[index] = TDrawSyncTokenRange(start, start + count - 1, pCallback);
        mLowTokenCount += count;
        return start;
    }

    u16 start = mHighTokenCount + 0xA000;
    mTokenRanges[index] = TDrawSyncTokenRange(start, start + count - 1, pCallback);
    mHighTokenCount += count;
    return start;
}


