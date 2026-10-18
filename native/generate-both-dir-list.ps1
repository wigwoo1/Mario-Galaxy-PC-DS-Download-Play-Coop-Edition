$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\Util\BothDirList.cpp" -Raw
$s=$s.Replace('#include "Game/Util.hpp"','#include "Game/Util/BothDirList.hpp"')
Set-Content "$n\src\generated\BothDirListKernel.cpp" $s
