$ErrorActionPreference='Stop'
$n=$PSScriptRoot
$p=Split-Path $n -Parent
$out=Join-Path $n 'src\generated\AreaObjCompat'
New-Item -ItemType Directory -Force $out | Out-Null

$source=Join-Path $p 'Petari\src\Game\AreaObj\AreaObjContainer.cpp'
$text=[System.IO.File]::ReadAllText($source)
$legacyBase='struct IsManagerName : std::binary_function< AreaObjMgr*, const char*, bool > {'
if([regex]::Matches($text,[regex]::Escape($legacyBase)).Count -ne 1){throw 'AreaObjContainer expected one removed MSL binary_function base'}
$text=$text.Replace($legacyBase,'struct IsManagerName {')
$legacyFind='return *std::find_if(mManagerArray, mManagerArray + mNumManagers, std::binder2nd< IsManagerName, const char* >(IsManagerName(), pName));'
if([regex]::Matches($text,[regex]::Escape($legacyFind)).Count -ne 1){throw 'AreaObjContainer expected one binder2nd manager lookup'}
$hostFind='return *std::find_if(mManagerArray, mManagerArray + mNumManagers, [pName](AreaObjMgr* pManager) { return IsManagerName()(pManager, pName); });'
$text=$text.Replace($legacyFind,$hostFind)
[System.IO.File]::WriteAllText((Join-Path $out 'AreaObjContainer.cpp'),$text,[System.Text.UTF8Encoding]::new($false))

$source=Join-Path $p 'Petari\src\Game\AreaObj\AreaObj.cpp'
$text=[System.IO.File]::ReadAllText($source)
$legacyInclude='#include <functional.hpp>'
if([regex]::Matches($text,[regex]::Escape($legacyInclude)).Count -ne 1){throw 'AreaObj expected one MSL functional header include'}
$text=$text.Replace($legacyInclude,'')
$legacySearch='(?s)    AreaObj\* const\* p = std::rfind_if\(.*?rVec\)\);'
$hostSearch=@'
    AreaObj* const* p = Begin - 1;
    for (AreaObj* const* it = End; it != Begin;) {
        --it;
        if ((*it)->isInVolume(rVec)) {
            p = it;
            break;
        }
    }
'@
if([regex]::Matches($text,$legacySearch).Count -ne 1){throw 'AreaObj expected one legacy reverse member-function search'}
$text=[regex]::Replace($text,$legacySearch,[System.Text.RegularExpressions.MatchEvaluator]{param($match)$hostSearch})
[System.IO.File]::WriteAllText((Join-Path $out 'AreaObj.cpp'),$text,[System.Text.UTF8Encoding]::new($false))

$source=Join-Path $p 'Petari\src\Game\AreaObj\ChangeBgmCube.cpp'
$text=[System.IO.File]::ReadAllText($source)
$switch='switch (objArg0) {'
if([regex]::Matches($text,[regex]::Escape($switch)).Count -ne 1){throw 'ChangeBgmCube expected one object-argument switch'}
$text=$text.Replace($switch,'switch (objArg0) {')
$caseZero='        case 0:'
if([regex]::Matches($text,[regex]::Escape($caseZero)).Count -ne 1){throw 'ChangeBgmCube expected one case 0 label'}
$text=$text.Replace($caseZero,'        case 0: {')
$caseOne='        case 1:'
if([regex]::Matches($text,[regex]::Escape($caseOne)).Count -ne 1){throw 'ChangeBgmCube expected one case 1 label'}
$text=$text.Replace($caseOne,"        }`r`n        case 1: {")
$caseTwo='        case 2:'
if([regex]::Matches($text,[regex]::Escape($caseTwo)).Count -ne 1){throw 'ChangeBgmCube expected one case 2 label'}
$text=$text.Replace($caseTwo,"        }`r`n        case 2:")
[System.IO.File]::WriteAllText((Join-Path $out 'ChangeBgmCube.cpp'),$text,[System.Text.UTF8Encoding]::new($false))
