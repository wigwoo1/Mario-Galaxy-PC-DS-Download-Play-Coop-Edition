$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\AreaObj\AreaObj.cpp" -Raw
$b=$s.IndexOf('AreaObj* AreaObjMgr::find_in(');$o=$s.IndexOf('{',$b);$d=1;$e=$o+1;while($d){if($s[$e] -eq '{'){$d++};if($s[$e] -eq '}'){$d--};$e++}
if($b -lt 0){throw 'Original area reverse lookup anchor missing'}
$replacement='AreaObj* AreaObjMgr::find_in(const TVec3f& rVec) const { auto begin=mArray.begin(); auto current=mArray.end(); while(current!=begin){--current;if((*current)->isInVolume(rVec))return *current;} return nullptr; }'
$s=$s.Substring(0,$b)+$replacement+$s.Substring($e)
$s=$s.Replace('#include <functional.hpp>','')
Set-Content "$n\src\generated\AreaObj.cpp" $s
$s=Get-Content "$p\Petari\src\Game\AreaObj\AreaObjContainer.cpp" -Raw
$s=$s.Replace('struct IsManagerName : std::binary_function< AreaObjMgr*, const char*, bool >','struct IsManagerName')
$old='return *std::find_if(mManagerArray, mManagerArray + mNumManagers, std::binder2nd< IsManagerName, const char* >(IsManagerName(), pName));'
if(!$s.Contains($old)){throw 'Original area manager lookup anchor missing'}
$s=$s.Replace($old,'auto found=std::find_if(mManagerArray,mManagerArray+mNumManagers,[pName](const AreaObjMgr* manager){return IsManagerName()(manager,pName);}); return found==mManagerArray+mNumManagers ? nullptr : *found;')
Set-Content "$n\src\generated\AreaObjContainer.cpp" $s

