$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\System\GameSystemException.cpp" -Raw
Set-Content "$n\src\generated\GameSystemException.cpp" ("#include <cstdarg>`r`nusing __va_list = va_list;`r`n"+$s)
