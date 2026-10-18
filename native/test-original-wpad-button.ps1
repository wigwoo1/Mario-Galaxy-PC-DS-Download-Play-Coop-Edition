$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
& "$n\generate-native-wpad-button.ps1"
& "$n\generate-native-wpad-stick.ps1"
& "$n\generate-camera-input-queries.ps1"
& "$n\generate-native-wpad-pointer.ps1"
$source="#include <Game/System/WPad.hpp>`n#include <Game/System/WPadHolder.hpp>`n#include <Game/System/WPadPointer.hpp>`n#include <Game/System/WPadAcceleration.hpp>`n#include <Game/System/WPadHVSwing.hpp>`n"
foreach($entry in @(@('WPad.cpp','s32 WPad::getValidStatusCount('),@('WPad.cpp','KPADStatus* WPad::getKPadStatus('),@('WPad.cpp','s32 WPad::getEnableAccelPastCount('),@('WPad.cpp','bool WPad::getAcceleration('),@('WPad.cpp','bool WPad::getPastAcceleration('),@('WPadHolder.cpp','u32 WPadReadDataInfo::getValidStatusCount('),@('WPadHolder.cpp','KPADStatus* WPadReadDataInfo::getKPadStatus('))){
 $s=Get-Content "$p\Petari\src\Game\System\$($entry[0])" -Raw;$start=$s.IndexOf($entry[1]);if($start -lt 0){throw 'Missing original accessor'}
 $brace=$s.IndexOf('{',$start);$end=$brace+1;$depth=1;while($depth){if($s[$end] -eq '{'){$depth++}elseif($s[$end] -eq '}'){$depth--};$end++}
 $source+=$s.Substring($start,$end-$start)+"`n"
}
$s=Get-Content "$p\Petari\src\Game\System\WPad.cpp" -Raw
$start=$s.IndexOf('    bool isDeviceFreeStyle(');$brace=$s.IndexOf('{',$start);$end=$s.IndexOf('}',$brace)+1
$source+="namespace MR {`n"+$s.Substring($start,$end-$start)+"`n}`n"
$source+="namespace { static const f32 sMoveJudgeSpeed=0.0001f; static const s32 sIsValidInScreenTime=5,sIsInValidOutScreenTime=10,sLostSensorTime=20; }"
$pointerSource=Get-Content "$n\src\generated\WPadPointer.cpp" -Raw
foreach($signature in @('WPadPointer::WPadPointer(', 'void WPadPointer::reset(', 'void WPadPointer::update(', 'void WPadPointer::getPointingPos(', 'void WPadPointer::getHorizonVec(', 'void WPadPointer::getPastPointingPos(', 's32 WPadPointer::getEnablePastCount(')){
 $start=$pointerSource.IndexOf($signature);if($start -lt 0){throw 'Missing original pointer accessor'}
 $brace=$pointerSource.IndexOf('{',$start);$end=$brace+1;$depth=1
 while($depth){if($pointerSource[$end] -eq '{'){$depth++}elseif($pointerSource[$end] -eq '}'){$depth--};$end++}
 $source+=$pointerSource.Substring($start,$end-$start)+"`n"
}
Set-Content "$n\src\generated\WPadButtonAccessors.cpp" $source
# Button fixture deliberately excludes pointer coordinate services; full candidate retains them.
$inputFixture=Get-Content "$n\src\generated\CameraInputQueries.cpp" -Raw
foreach($signature in @('    void getCorePadPointingPosBasedOnScreen(')){
 $start=$inputFixture.IndexOf($signature);if($start -lt 0){throw 'Missing pointer query'}
 $brace=$inputFixture.IndexOf('{',$start);$end=$inputFixture.IndexOf('}',$brace)+1
 $inputFixture=$inputFixture.Remove($start,$end-$start)
}
Set-Content "$n\src\generated\ButtonInputQueriesFixture.cpp" $inputFixture
$env:ZIG_GLOBAL_CACHE_DIR="$p\tools\zig-cache"
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -UNDEBUG -ffunction-sections -fdata-sections "-Wl,--gc-sections" -target x86_64-windows-gnu "-I$n\include-runtime" "-I$n\include" "-I$p\Petari\include" "-I$p\Petari\libs\JSystem\include" "-I$p\Petari\libs\RVL_SDK\include" -include revolution/types.h -include JSystem/JMath/JMath.hpp -include "$n\include-runtime\revolution\os.h" -include "$n\include-runtime\revolution\gx\GXVert.h" "$n\src\native_kpad_input.cpp" "$n\src\original_wpad_button_tests.cpp" "$n\src\generated\WPadButtonAccessors.cpp" "$n\src\generated\WPadButton.cpp" "$n\src\generated\WPadStick.cpp" "$n\src\generated\ButtonInputQueriesFixture.cpp" "$n\src\generated\WPadCompat\WPadAcceleration.cpp" "$p\Petari\src\Game\System\WPadHVSwing.cpp" -o "$n\build\original-wpad-button-tests.exe"
if($LASTEXITCODE){exit $LASTEXITCODE}
& "$n\build\original-wpad-button-tests.exe"
exit $LASTEXITCODE











