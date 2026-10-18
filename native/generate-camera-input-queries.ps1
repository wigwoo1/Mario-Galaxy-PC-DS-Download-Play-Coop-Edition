$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\Util\GamePadUtil.cpp" -Raw
$body=$s.Substring(0,$s.IndexOf('void GamePadUtil_FORCE_MATCH_SDATA2'))+"`nnamespace MR {`n"
foreach($signature in @('    bool isGamePadStickOperated(', '    void getCorePadPointingPosBasedOnScreen(', '    void getCorePadPointingPos(', '    void getCorePadPastPointingPos(', '    s32 getCorePadEnablePastCount(', '    bool isCorePadPointInScreen(', '    bool isConnectedWPad(', '    f32 getCorePadDistanceToDisplay(', '    bool testSubPadButtonZ(', '    bool testCorePadButtonUp(', '    bool testCorePadButtonDown(', '    bool testCorePadButtonLeft(', '    bool testCorePadButtonRight(', '    bool testCorePadButtonA(', '    bool testCorePadButtonB(', '    bool testCorePadButton1(', '    bool testCorePadButton2(', '    bool testCorePadButtonPlus(', '    bool testCorePadButtonMinus(', '    bool testSubPadButtonC(', '    bool testPadButtonAnyWithoutHome(', '    bool testCorePadTriggerA(', '    bool testCorePadTriggerUp(', '    bool testCorePadTriggerDown(', '    bool testCorePadTriggerLeft(', '    bool testCorePadTriggerRight(', '    bool testCorePadTriggerB(', '    bool testCorePadTrigger1(', '    bool testCorePadTrigger2(', '    bool testCorePadTriggerPlus(', '    bool testCorePadTriggerMinus(', '    bool testCorePadTriggerAnyWithoutHome(')){
 $start=$s.IndexOf($signature);if($start -lt 0){throw 'Missing pinned input helper'}
 $brace=$s.IndexOf('{',$start);$end=$brace+1;$depth=1
 while($depth){if($s[$end] -eq '{'){$depth++}elseif($s[$end] -eq '}'){$depth--};$end++}
 $body+=$s.Substring($start,$end-$start)+"`n"
}
$body+="}`n"
Set-Content "$n\src\generated\CameraInputQueries.cpp" $body






