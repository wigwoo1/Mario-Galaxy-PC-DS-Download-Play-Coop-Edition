$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\Screen\odh.cpp" -Raw
$s='#include <cstdint>'+"`n"+$s.Replace('reinterpret_cast< u32 >(pCoefficients)','reinterpret_cast< std::uintptr_t >(pCoefficients)').Replace('u16 size[2] = {width, height};','u16 size[2] = {static_cast<u16>(width), static_cast<u16>(height)};')
$header='    reinterpret_cast< u32* >(pMaster->mOutput)[1] = (pMaster->mQuality << 24) | (pMaster->mSize[0] | (pMaster->mSize[1] << 11));'
if(!$s.Contains($header)){throw 'Original AJPG header anchor missing'}
$s=$s.Replace($header,'    auto writeBE = [&](unsigned offset, u32 value){for(unsigned i=0;i<4;i++)pMaster->mOutput[offset+i]=u8(value>>(24-i*8));};'+"`n"+'    writeBE(4,(u32(pMaster->mQuality) << 24) | (pMaster->mSize[0] | (u32(pMaster->mSize[1]) << 11)));')
$s=$s.Replace('reinterpret_cast< u32* >(pMaster->mOutput)[2] = size;','writeBE(8,size);').Replace('reinterpret_cast< u32* >(pMaster->mOutput)[3] = 0;','writeBE(12,0);')
$anchor='        pMaster->mDct[0] = 0x40004000;'
if(([regex]::Matches($s,[regex]::Escape($anchor))).Count -ne 2){throw 'Original Huffman coefficient handoff anchors missing'}
$s=$s.Replace($anchor,'        // Preserve Wii halfword traversal in packed coefficient words on little-endian native host.'+"`n"+'        for(unsigned i=0;i<64;i++){u32 value=pMaster->mCoefficients[i];pMaster->mCoefficients[i]=(value<<16)|(value>>16);}'+"`n"+$anchor)
Set-Content "$n\src\generated\odh.cpp" $s


