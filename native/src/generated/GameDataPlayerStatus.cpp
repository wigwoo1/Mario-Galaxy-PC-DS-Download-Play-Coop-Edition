#include <cstring>
#include "Game/System/GameDataPlayerStatus.hpp"
#include "Game/Util/MathUtil.hpp"
#include <JSystem/JSupport/JSUMemoryInputStream.hpp>
#include <JSystem/JSupport/JSUMemoryOutputStream.hpp>

GameDataPlayerStatus::GameDataPlayerStatus() {
    initializeData();
}

u32 GameDataPlayerStatus::makeHeaderHashCode() const {
    return 0x27C90F;
}

u32 GameDataPlayerStatus::getSignature() const {
    return 'PLAY';
}

s32 GameDataPlayerStatus::serialize(u8* buffer, u32 size) const {
    if (!buffer || size < 7) return -1;
    const u8 bytes[7] = {mStoryProgress,
        u8(mStockedStarPiece >> 24), u8(mStockedStarPiece >> 16),
        u8(mStockedStarPiece >> 8), u8(mStockedStarPiece),
        u8(mPlayerLeft >> 8), u8(mPlayerLeft)};
    std::memcpy(buffer, bytes, sizeof(bytes));
    return 7;
}
s32 GameDataPlayerStatus::deserialize(const u8* buffer, u32 size) {
    // Original legacy records contain only progress and stocked Star Bits.
    // A partial two-byte supply field is invalid; failure preserves state.
    if (!buffer || size < 5 || size == 6) return -1;
    const u8 progress = buffer[0];
    const u32 stocked = (u32(buffer[1]) << 24) | (u32(buffer[2]) << 16)
                      | (u32(buffer[3]) << 8) | u32(buffer[4]);
    const u16 supply = size >= 7 ? u16((u16(buffer[5]) << 8) | buffer[6]) : u16(4);
    initializeData();
    mStoryProgress = progress;
    mStockedStarPiece = stocked;
    mPlayerLeftSupply = supply;
    mPlayerLeft = 4;
    return 0;
}
void GameDataPlayerStatus::initializeData() {
    mPlayerLeft = 4;
    mPlayerLeftSupply = 4;
    mStockedStarPiece = 0;
    mStoryProgress = 0;
}

s32 GameDataPlayerStatus::getPlayerLeft() const {
    s32 playerLeft = mPlayerLeft;

    return MR::clamp(playerLeft, 0, 99);
}

void GameDataPlayerStatus::addPlayerLeft(int num) {
    s32 playerLeft = mPlayerLeft + num;

    mPlayerLeft = MR::clamp(playerLeft, 0, 99);
}

bool GameDataPlayerStatus::isPlayerLeftSupply() const {
    return mPlayerLeftSupply >= 10;
}

void GameDataPlayerStatus::offPlayerLeftSupply() {
    mPlayerLeftSupply = 0;
}

void GameDataPlayerStatus::addStockedStarPiece(int num) {
    s32 stockedStarPiece = mStockedStarPiece + num;

    mStockedStarPiece = MR::clamp(stockedStarPiece, 0, 9999);
}

