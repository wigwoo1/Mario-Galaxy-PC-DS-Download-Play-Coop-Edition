$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\System\GameSystemStationedArchiveLoader.cpp" -Raw
$s=$s.Replace('namespace {',"#undef ATTRIBUTE_WEAK`n#define ATTRIBUTE_WEAK`nnamespace {")
$s=$s.Replace('alloc(0x10000, nullptr)','alloc(0x10000, 0)')
Set-Content "$n\src\generated\GameSystemStationedArchiveLoader.cpp" $s
$header=Get-Content "$p\Petari\include\Game\System\StationedArchiveLoader.hpp" -Raw
$header=$header.Replace('virtual bool isExecute(const MR::StationedFileInfo*) const;','virtual bool isExecute(const MR::StationedFileInfo*) const = 0;')
Set-Content "$n\include-runtime\Game\System\StationedArchiveLoader.hpp" $header
# Compile the actual concrete policy methods without unfinished controller deps.
$policy=Get-Content "$p\Petari\src\Game\System\GameSystemStationedArchiveLoader.cpp" -Raw
$a=$policy.IndexOf('bool ConditionIfIsNotPlayer::isExecute(')
$b=$policy.IndexOf('PlayerHeapHolder::PlayerHeapHolder()', $a)
$c=$policy.IndexOf('bool ConditionIsEqualType::isExecute(')
if($a -lt 0 -or $b -lt 0 -or $c -lt 0){throw 'Policy extraction anchors missing'}
$base=Get-Content "$p\Petari\src\Game\System\StationedArchiveLoader.cpp" -Raw
$d=$base.IndexOf('StationedArchiveLoader::Condition::~Condition()')
$e=$base.IndexOf('JKRHeap* StationedArchiveLoader::getProperHeap(', $d)
$prefix='#include <JSystem/JKernel/JKRExpHeap.hpp>'+[Environment]::NewLine+'#include <Game/System/GameSystemStationedArchiveLoader.hpp>'+[Environment]::NewLine+'#include <Game/System/StationedFileInfo.hpp>'+[Environment]::NewLine
Set-Content "$n\src\generated\StationedConditionsFixture.cpp" ($prefix+$base.Substring($d,$e-$d)+$policy.Substring($a,$b-$a)+$policy.Substring($c))

Set-Content "$n\src\generated\StationedArchiveLoader.cpp" ('#include <type_traits>'+[Environment]::NewLine+$base+[Environment]::NewLine+'static_assert(std::is_abstract<StationedArchiveLoader::Condition>::value, "Native abstract archive condition header required");')
