$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\NameObj\NameObjArchiveListCollector.cpp" -Raw
$anchor='    MR::copyString(mArchive[mArchiveNum], pName, ARRAY_SIZE(*mArchive));'
if(!$s.Contains($anchor)){throw 'Archive collector add anchor missing'}
$s=$s.Replace($anchor,'    if (!pName || std::strlen(pName) >= sizeof(mArchive[0])) throw std::invalid_argument("Invalid native object archive name");'+[Environment]::NewLine+'    if (mArchiveNum >= ARRAY_SIZE(mArchive)) throw std::length_error("Object archive collector capacity exceeded");'+[Environment]::NewLine+$anchor)
$s=$s.Replace('    return mArchive[index];','    if (index < 0 || index >= mArchiveNum) throw std::out_of_range("Object archive index");'+[Environment]::NewLine+'    return mArchive[index];')
Set-Content "$n\src\generated\NameObjArchiveListCollector.cpp" ('#include <cstring>'+[Environment]::NewLine+'#include <stdexcept>'+[Environment]::NewLine+$s)
