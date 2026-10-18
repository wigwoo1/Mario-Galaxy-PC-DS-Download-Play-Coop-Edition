$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent;$s=Get-Content -Raw "$p\Petari\src\JSystem\JUtility\JUTTexture.cpp"
$body=''
foreach($signature in @('void JUTTexture::capture(')){
 $start=$s.IndexOf($signature);if($start -lt 0){throw 'Original texture sizing anchor missing'};$brace=$s.IndexOf('{',$start);$depth=1;$end=$brace+1;while($depth){if($s[$end] -eq '{'){$depth++}elseif($s[$end] -eq '}'){$depth--};$end++};$body+=$s.Substring($start,$end-$start)+"`n"
}
Set-Content "$n\src\generated\JUTTextureCaptureKernel.cpp" ('#include <JSystem/JUtility/JUTTexture.hpp>'+"`n"+$body)

