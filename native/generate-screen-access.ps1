$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content -Raw "$p\Petari\src\Game\Util\ScreenUtil.cpp"
function Extract-ScreenBody([string]$signature){
 $start=$s.IndexOf($signature);if($start -lt 0){throw "Screen accessor anchor missing: $signature"};$brace=$s.IndexOf('{',$start);$depth=1;$end=$brace+1;while($depth){if($s[$end] -eq '{'){$depth++}elseif($s[$end] -eq '}'){$depth--};$end++};return $s.Substring($start,$end-$start)
}
$body=@"
#include <Game/Util/ScreenUtil.hpp>
#include <Game/Util/SystemUtil.hpp>
#include <Game/Util/SingletonHolder.hpp>
#include <Game/System/GameSystem.hpp>
#include <Game/System/GameSystemObjHolder.hpp>
#include <Game/Screen/CaptureScreenDirector.hpp>
"@
$body += "`nnamespace {`n"+(Extract-ScreenBody 'CaptureScreenDirector* getCaptureScreenDirector()')+"`n}`nnamespace MR {`n"
foreach($signature in @('s32 getScreenWidth()','void startToCaptureScreen(','void endToCaptureScreen(','void captureScreenIfAllow(','const ResTIMG* getScreenResTIMG()','u8* getScreenTexImage()')){$body+=(Extract-ScreenBody $signature)+"`n"}
$body+="}`n";Set-Content "$n\src\generated\ScreenAccessKernel.cpp" $body
