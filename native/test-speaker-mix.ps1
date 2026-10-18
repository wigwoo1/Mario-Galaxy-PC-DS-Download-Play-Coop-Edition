$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\Speaker\SpkMixingBuffer.cpp" -Raw
$body="#include <Game/Speaker/SpkMixingBuffer.hpp>`n#include <JSystem/JAudio2/JASCalc.hpp>`n"
foreach($signature in @('void SpkMixingBuffer::mix(', 'const s16* SpkMixingBuffer::getSamples(')){
 $start=$s.IndexOf($signature);if($start -lt 0){throw 'Missing original speaker mix'}
 $brace=$s.IndexOf('{',$start);$end=$brace+1;$depth=1
 while($depth){if($s[$end] -eq '{'){$depth++}elseif($s[$end] -eq '}'){$depth--};$end++}
 $body+=$s.Substring($start,$end-$start)+"`n"
}
$body+="#include <Game/Speaker/SpkSound.hpp>"
$s=Get-Content "$p\Petari\src\Game\Speaker\SpkSound.cpp" -Raw
foreach($signature in @('SpkSoundVolume::SpkSoundVolume(', 'void SpkSoundVolume::setRelease(', 'f32 SpkSoundVolume::calc(')){
 $start=$s.IndexOf($signature);if($start -lt 0){throw 'Missing original speaker volume'}
 $brace=$s.IndexOf('{',$start);$end=$brace+1;$depth=1
 while($depth){if($s[$end] -eq '{'){$depth++}elseif($s[$end] -eq '}'){$depth--};$end++}
 $body+=[Environment]::NewLine+$s.Substring($start,$end-$start)+[Environment]::NewLine
}
Set-Content "$n\src\generated\SpeakerMixFixture.cpp" $body
$env:ZIG_GLOBAL_CACHE_DIR="$p\tools\zig-cache"
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -UNDEBUG -target x86_64-windows-gnu "-I$n\include-runtime" "-I$n\include" "-I$p\Petari\include" "-I$p\Petari\libs\JSystem\include" "-I$p\Petari\libs\RVL_SDK\include" -include revolution/types.h -include "$n\include-runtime\revolution\os.h" "$n\src\speaker_mix_tests.cpp" "$n\src\generated\SpeakerMixFixture.cpp" -o "$n\build\speaker-mix-tests.exe" *> "$n\build\speaker-mix-tests.log"
if($LASTEXITCODE){Get-Content "$n\build\speaker-mix-tests.log" -Tail 15;exit $LASTEXITCODE}
& "$n\build\speaker-mix-tests.exe"
exit $LASTEXITCODE

