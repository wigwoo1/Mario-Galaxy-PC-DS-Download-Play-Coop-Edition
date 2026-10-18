$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$h=Get-Content "$p\Petari\libs\JSystem\include\JSystem\JSupport\JSUInputStream.hpp" -Raw
Set-Content "$n\include\JSystem\JSupport\JSUInputStream.hpp" ($h.Replace('virtual ~JSUInputStream() ATTRIBUTE_WEAK;','virtual ~JSUInputStream();'))
$h=Get-Content "$p\Petari\libs\JSystem\include\JSystem\JSupport\JSUOutputStream.hpp" -Raw
Set-Content "$n\include\JSystem\JSupport\JSUOutputStream.hpp" ($h.Replace('virtual ~JSUOutputStream() ATTRIBUTE_WEAK;','virtual ~JSUOutputStream();'))
