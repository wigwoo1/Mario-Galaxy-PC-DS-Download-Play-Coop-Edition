$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent;$env:ZIG_GLOBAL_CACHE_DIR="$p\tools\zig-cache"
& "$n\generate-actor-runtime.ps1"
$kernel="#include <Game/Animation/XanimeCore.hpp>`n#include <Game/Util/LiveActorUtil.hpp>`n"
foreach($spec in ,@("$p\Petari\src\Game\Animation\XanimeCore.cpp",'XjointTransform::XjointTransform()')){
$text=[IO.File]::ReadAllText($spec[0]);$b=$text.IndexOf($spec[1]);if($b -lt 0){throw 'actor adaptation method anchor'};$o=$text.IndexOf('{',$b);$d=1;$e=$o+1;while($d){if($text[$e] -eq '{'){$d++};if($text[$e] -eq '}'){$d--};$e++};$body=$text.Substring($b,$e-$b);if($spec[1].StartsWith('void')){$body="namespace MR {`n$body`n}"};$kernel+=$body+"`n"
}
[IO.File]::WriteAllText("$n\src\generated\ActorAdaptationKernel.cpp",$kernel)
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -O2 -UNDEBUG -target x86_64-windows-gnu "-I$n\include-runtime" "-I$n\include" "-I$p\Petari\include" "-I$p\Petari\libs\JSystem\include" "-I$p\Petari\libs\RVL_SDK\include" "-I$p\Petari\libs\nw4r\include" -include revolution/types.h -include "$n\include-runtime\revolution\os.h" -include "$n\include-runtime\revolution\gx\GXVert.h" -include "$n\include-runtime\revolution\os\OSTime.h" "$n\src\actor_native_adaptation_tests.cpp" "$n\src\generated\ActorAdaptationKernel.cpp" "$p\Petari\src\Game\LiveActor\LiveActorFlag.cpp" -o "$n\build\actor-native-adaptation-tests.exe" *> "$n\build\actor-native-adaptation-tests.log"
if($LASTEXITCODE){Get-Content "$n\build\actor-native-adaptation-tests.log" -Tail 20;exit 1}
& "$n\build\actor-native-adaptation-tests.exe"
exit $LASTEXITCODE