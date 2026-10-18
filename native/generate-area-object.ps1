$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\AreaObj\AreaObj.cpp" -Raw
$s=$s.Replace('#include <functional.hpp>','#include <Native/AreaSearch.hpp>')
$signature='AreaObj* AreaObjMgr::find_in('
$start=$s.IndexOf($signature);if($start -lt 0){throw 'Original area reverse search anchor missing'}
$brace=$s.IndexOf('{',$start);$depth=1;$end=$brace+1;while($depth){if($s[$end] -eq '{'){$depth++}elseif($s[$end] -eq '}'){$depth--};$end++}
$body=@'
AreaObj* AreaObjMgr::find_in(const TVec3f& point) const {
    // Preserve the original last-entry-first search without forming before-begin pointers.
    return Native::findLastArea(mArray.begin(),mArray.size(),[&](AreaObj* area){return area->isInVolume(point);});
}
'@
$s=$s.Substring(0,$start)+$body+$s.Substring($end)
Set-Content "$n\src\generated\AreaObjKernel.cpp" $s
