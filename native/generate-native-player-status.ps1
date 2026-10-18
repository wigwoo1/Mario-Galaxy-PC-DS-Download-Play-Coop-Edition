$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\System\GameDataPlayerStatus.cpp" -Raw
$a=$s.IndexOf('s32 GameDataPlayerStatus::serialize(')
$b=$s.IndexOf('void GameDataPlayerStatus::initializeData()', $a)
if($a -lt 0 -or $b -lt 0){throw 'Player save serializer anchors missing'}
$codec=@"
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

"@
Set-Content "$n\src\generated\GameDataPlayerStatus.cpp" ('#include <cstring>'+[Environment]::NewLine+$s.Substring(0,$a)+$codec+$s.Substring($b))
