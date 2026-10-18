$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$source='#include "../native_jmap_storage.hpp"'+[Environment]::NewLine+'#include "Native/BuiltinTables.hpp"'+[Environment]::NewLine
foreach($entry in @(@('GalaxyID','galaxyIdTable'),@('StoryEvent','storyEventTable'))){
 $bytes=[IO.File]::ReadAllBytes("$p\Petari\src\Game\System\$($entry[0]).bcsv")
 if($bytes.Length -lt 16){throw 'Missing built-in table'}
 $source+='namespace {const std::uint8_t '+$entry[0]+'Bytes[]={'+($bytes -join ',')+'};}'+[Environment]::NewLine
 $source+='const void* Native::'+$entry[1]+'(){static const NativeJMapStorage table(std::vector<std::uint8_t>('+ $entry[0]+'Bytes,'+$entry[0]+'Bytes+sizeof('+$entry[0]+'Bytes)));return table.data();}'+[Environment]::NewLine
}
Set-Content "$n\src\generated\NativeBuiltinTables.cpp" $source
foreach($unit in @('GameDataConst','GameEventFlagTable')){
 $s=Get-Content "$p\Petari\src\Game\System\$unit.cpp" -Raw
 $s='#include "Native/BuiltinTables.hpp"'+[Environment]::NewLine+$s.Replace('&GalaxyIDBCSV','Native::galaxyIdTable()')
 Set-Content "$n\src\generated\$unit.cpp" $s
}
$path="$n\src\generated\GameDataHolder.cpp"
$s=Get-Content $path -Raw
$s=$s.Replace('attach(StoryEventBCSV)','attach(Native::storyEventTable())')
if(!$s.Contains('#include "Native/BuiltinTables.hpp"')){$s='#include "Native/BuiltinTables.hpp"'+[Environment]::NewLine+$s}
Set-Content $path $s
