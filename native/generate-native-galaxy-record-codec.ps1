$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\System\GameDataGalaxyStorage.cpp" -Raw
$start=$s.IndexOf('s32 GameDataSomeGalaxyStorage::deserialize(')
$end=$s.IndexOf('GameDataAllGalaxyStorage::GameDataAllGalaxyStorage()', $start)
$decode=@"
s32 GameDataSomeGalaxyStorage::deserialize(const BinaryDataContentAccessor& a,const u8* data) {
    if (!data) return -1;
    u8* stars=(u8*)a.getPointer("mPowerStarFlag",(u8*)data);
    u8* visited=(u8*)a.getPointer("mFirstPlayFlag",(u8*)data);
    u8* coins=(u8*)a.getPointer("mMaxCoinNum",(u8*)data);
    if (coins && (a.getDataSize()<16 || coins-data>a.getDataSize()-16)) return -1;
    u16 decoded[8]{};
    if(coins) for(int i=0;i<8;++i) decoded[i]=u16((u16(coins[i*2])<<8)|coins[i*2+1]);
    mPowerStarOwnedFlags=stars?*stars:0;
    mAlreadyVisitedFlags=visited?*visited:0;
    for(int i=0;i<8;++i)mMaxCoinNum[i]=decoded[i];
    return stars&&visited&&coins?0:1;
}

"@
$s=$s.Substring(0,$start)+$decode+$s.Substring($end)
$start=$s.IndexOf('void GameDataSomeGalaxyStorage::serialize(')
$end=$s.IndexOf('s32 GameDataAllGalaxyStorage::serialize(', $start)
$encode=@"
void GameDataSomeGalaxyStorage::serialize(const BinaryDataContentAccessor& a,u8* data) const {
    if(!data) throw std::invalid_argument("Null galaxy record");
    u8* stars=(u8*)a.getPointer("mPowerStarFlag",data);
    u8* visited=(u8*)a.getPointer("mFirstPlayFlag",data);
    u8* coins=(u8*)a.getPointer("mMaxCoinNum",data);
    if(!stars||!visited||!coins||a.getDataSize()<16||coins-data>a.getDataSize()-16)
        throw std::length_error("Incomplete galaxy record fields");
    *stars=mPowerStarOwnedFlags;*visited=mAlreadyVisitedFlags;
    for(int i=0;i<8;++i){coins[i*2]=u8(mMaxCoinNum[i]>>8);coins[i*2+1]=u8(mMaxCoinNum[i]);}
}

"@
$s=$s.Substring(0,$start)+$encode+$s.Substring($end)
$s='#include <stdexcept>'+[Environment]::NewLine+$s
Set-Content "$n\src\generated\GameDataGalaxyStorage.cpp" $s
# Extract actual record methods for isolated codec verification, without unrelated all-galaxy services.
$fixture='#include <Game/System/GameDataGalaxyStorage.hpp>'+[Environment]::NewLine+'#include <Game/System/BinaryDataContentAccessor.hpp>'+[Environment]::NewLine+'#include <Game/System/GalaxyStatusAccessor.hpp>'+[Environment]::NewLine+'#include <stdexcept>'+[Environment]::NewLine
foreach($method in @('GameDataSomeGalaxyStorage::GameDataSomeGalaxyStorage','void GameDataSomeGalaxyStorage::resetAllData','s32 GameDataSomeGalaxyStorage::deserialize','void GameDataSomeGalaxyStorage::serialize')){
 $a=$s.IndexOf($method);$b=$s.IndexOf('{',$a);$depth=1;$e=$b+1
 while($depth){if($s[$e] -eq '{'){$depth++};if($s[$e] -eq '}'){$depth--};$e++}
 $fixture+=$s.Substring($a,$e-$a)+[Environment]::NewLine
}
Set-Content "$n\src\generated\GalaxyRecordCodecFixture.cpp" $fixture
