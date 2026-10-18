$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$source=Get-Content "$p\Petari\src\JSystem\JAudio2\JASProbe.cpp" -Raw
Set-Content "$n\src\generated\AudioProbeKernel.cpp" ($source.Replace('if (index < 16)','if (index >= 0 && index < 16)'))
