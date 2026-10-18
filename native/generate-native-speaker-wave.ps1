$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\Speaker\SpkWave.cpp" -Raw
$anchor='return (WaveData*)((u32)mResource + *(u32*)((u32)mResource + wave * 4 + 8));'
if(!$s.Contains($anchor)){throw 'Missing original wave pointer expression'}
$s=$s.Replace($anchor,'auto base=static_cast<unsigned char*>(mResource);u32 offset;std::memcpy(&offset,base+static_cast<size_t>(wave)*4+8,sizeof(offset));return reinterpret_cast<WaveData*>(base+offset);')
$s='#include <cstring>'+[Environment]::NewLine+$s
Set-Content "$n\src\generated\SpkWave.cpp" $s
$fixture="#include <Game/Speaker/SpkWave.hpp>`n#include <cstring>`n"
foreach($signature in @('SpkWave::SpkWave(', 's32 SpkWave::getWaveSize(', 'u32 SpkWave::getLoopStartPos(', 'u32 SpkWave::getLoopEndPos(', 's16* SpkWave::getWave(', 'WaveData* SpkWave::getWaveData(')){
 $start=$s.IndexOf($signature);if($start -lt 0){throw 'Missing wave getter'}
 $brace=$s.IndexOf('{',$start);$end=$brace+1;$depth=1
 while($depth){if($s[$end] -eq '{'){$depth++}elseif($s[$end] -eq '}'){$depth--};$end++}
 $fixture+=$s.Substring($start,$end-$start)+[Environment]::NewLine
}
Set-Content "$n\src\generated\SpeakerWaveGettersFixture.cpp" $fixture
