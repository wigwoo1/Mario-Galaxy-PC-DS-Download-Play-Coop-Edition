$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\JSystem\JAudio2\JAUAudioArcLoader.cpp" -Raw
$s='#include <JSystem/JAudio2/JAUSoundTable.hpp>'+[Environment]::NewLine+$s
$s=$s.Replace('mSection->newSoundTable(pData, 0, true);', 'auto* table = mSection->newSoundTable(pData, 0, true);'+[Environment]::NewLine+'    if (table) table->nativeResourceSize = param_1;')
$s=$s.Replace('void const* data = mSection->loadDVDFile(pFileName, true, EXPAND_SWITCH_UNKNOWN0);', 'u32 nativeSize=0;'+[Environment]::NewLine+'    void const* data = JKRDvdToMainRam(pFileName, nullptr, EXPAND_SWITCH_UNKNOWN1, 0, mSection->getHeap_(), JKRDvdRipper::ALLOC_DIRECTION_BACKWARD, 0, nullptr, &nativeSize);')
$s=$s.Replace('return data != nullptr && load(data);', 'return data != nullptr && parse(data, nativeSize);')
Set-Content "$n\src\generated\AudioArcLoaderKernel.cpp" $s


