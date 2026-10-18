$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\Util\SoundUtil.cpp" -Raw
$body=$s.Substring(0,$s.IndexOf('bool SoundUtil_FORCE_MATCH_DATA'))+"namespace MR {`n"
foreach($signature in @('    void startCSSound(', '    void startCSSound2P(')){
 $start=$s.IndexOf($signature);if($start -lt 0){throw 'Missing original pointer sound route'}
 $brace=$s.IndexOf('{',$start);$end=$brace+1;$depth=1
 while($depth){if($s[$end] -eq '{'){$depth++}elseif($s[$end] -eq '}'){$depth--};$end++}
 $body+=$s.Substring($start,$end-$start)+"`n"
}
Set-Content "$n\src\generated\PointerSoundRouting.cpp" ($body+"}`n")
