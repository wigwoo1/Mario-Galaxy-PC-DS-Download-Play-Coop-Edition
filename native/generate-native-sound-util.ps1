$ErrorActionPreference='Stop'
$n=$PSScriptRoot
function Remove-Definition([string]$source,[string]$signature){
 $start=$source.IndexOf($signature);if($start -lt 0){throw "Missing definition $signature"}
 $open=$source.IndexOf('{',$start);$depth=1;$end=$open+1
 while($depth -gt 0 -and $end -lt $source.Length){if($source[$end] -eq '{'){$depth++};if($source[$end] -eq '}'){$depth--};$end++}
 if($depth){throw "Unbalanced definition $signature"}
 return $source.Remove($start,$end-$start)
}
$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\Util\SoundUtil.cpp" -Raw
foreach($signature in @('s32 getMapSoundCodeFoot(const LiveActor* pActor)','void startBas(const LiveActor* pActor, const char* pName, bool param3, f32 param4, f32 param5)','void actorSoundMovement(LiveActor* pActor)','void startCSSound(const char* pCSName, const char* pSEName, s32 param3)','void startCSSound2P(const char* pCSName, const char* pSEName)')){
 $s=Remove-Definition $s $signature
}
Set-Content "$n\src\generated\SoundUtil.cpp" $s
