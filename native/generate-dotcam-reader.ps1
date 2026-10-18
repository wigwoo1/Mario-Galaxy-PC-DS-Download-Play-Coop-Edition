$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$headers=Join-Path $n 'include-runtime\Game\Camera';New-Item -ItemType Directory -Force -Path $headers | Out-Null
$h=Get-Content -LiteralPath (Join-Path $p 'Petari\include\Game\Camera\DotCamParams.hpp') -Raw
foreach($pair in @(@('DotCamReaderInBin(const void*);','DotCamReaderInBin(const void*, s32 size);'),@('void init(const void*);','void init(const void*, s32 size);'))){
 if(-not $h.Contains($pair[0])){throw 'DotCam header anchor missing.'};$h=$h.Replace($pair[0],$pair[1])
}
[IO.File]::WriteAllText((Join-Path $headers 'DotCamParams.hpp'),$h)
$s=Get-Content -LiteralPath (Join-Path $p 'Petari\src\Game\Camera\DotCamParams.cpp') -Raw
$s=$s.Replace('DotCamReaderInBin::DotCamReaderInBin(const void* pData)','DotCamReaderInBin::DotCamReaderInBin(const void* pData, s32 size)').Replace('    init(pData);','    init(pData, size);').Replace('void DotCamReaderInBin::init(const void* pData)','void DotCamReaderInBin::init(const void* pData, s32 size)')
$anchor='    mMapInfo.attach(pData);'
if(-not $s.Contains($anchor)){throw 'Raw DotCam attachment anchor missing.'}
$s=$s.Replace($anchor,@'
    if (!pData || size < 16) throw std::invalid_argument("Missing or truncated camera parameter table");
    const auto* first=static_cast<const std::uint8_t*>(pData);
    mMapInfo.attachNative(std::vector<std::uint8_t>(first,first+size));
'@)
$s=$s.Replace('    mMapInfo.begin().getValue("version", &mVersion);','    mVersion=0;'+"`n"+'    if(mMapInfo.getNumEntries())mMapInfo.begin().getValue("version", &mVersion);')
$s="#include <stdexcept>`n#include <vector>`n#include <cstdint>`n"+$s
[IO.File]::WriteAllText((Join-Path $n 'src\generated\DotCamParams.cpp'),$s)
$caller=Get-Content -LiteralPath (Join-Path $p 'Petari\src\Game\Camera\CameraParamChunkHolder.cpp') -Raw
$anchor='DotCamReaderInBin(data);';if(-not $caller.Contains($anchor)){throw 'Camera table extent call anchor missing.'}
$caller=$caller.Replace($anchor,'DotCamReaderInBin(data, local44);'+"`n"+'        Native::retainHeapJMap(reader.mMapInfo, MR::getSceneHeapNapa());')
$caller='#include <Native/HeapJMapOwner.hpp>'+[Environment]::NewLine+'#include <Game/Util/MemoryUtil.hpp>'+[Environment]::NewLine+'#include <JSystem/JKernel/JKRSolidHeap.hpp>'+"`n"+$caller
[IO.File]::WriteAllText((Join-Path $n 'src\generated\CameraParamChunkHolder.cpp'),$caller)
