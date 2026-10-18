$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\Util\MapPartsUtil.cpp" -Raw
$start=$s.IndexOf('    void getMapPartsObjectName(char* pBuffer, u32 size, const JMapInfoIter& rIter)')
$end=$s.IndexOf('    void initMapPartsClipping(', $start)
if($start -lt 0 -or $end -lt 0){throw 'Original map-part naming function boundaries missing'}
$prefix=@"
#include <cstdio>
#include "Game/Util/MapPartsUtil.hpp"
#include "Game/Util/JMapUtil.hpp"
namespace MR {
"@
($prefix+"`r`n"+$s.Substring($start,$end-$start)+"`r`n}`r`n")|Set-Content "$n\src\generated\MapPartsArchiveNameKernel.cpp"
