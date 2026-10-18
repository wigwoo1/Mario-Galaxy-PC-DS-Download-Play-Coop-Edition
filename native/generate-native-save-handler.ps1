$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\System\SaveDataHandler.cpp" -Raw
$s=$s.Replace('namespace {'+[Environment]::NewLine+'    NEW_NERVE(SaveDataHandlerWait','namespace NativeSaveHandlerNerves {'+[Environment]::NewLine+'    NEW_NERVE(SaveDataHandlerWait').Replace('GET_NERVE_ANON(', 'GET_NERVE_DIRECT(NativeSaveHandlerNerves, ')
foreach($signature in @('bool SaveDataHandler::requestVerifyAfterLoadGameDataFile(', 'void SaveDataHandler::requestSaveSaveData(', 'void SaveDataHandler::copySaveDataEachFile(', 'void SaveDataHandler::restoreGameDataFile(')) {
$a=$s.IndexOf($signature);if($a -lt 0){throw "Missing $signature"};$b=$s.IndexOf('{',$a);$depth=1;$end=$b+1
while($depth){if($s[$end] -eq '{'){$depth++};if($s[$end] -eq '}'){$depth--};$end++}
$s=$s.Remove($a,$end-$a)
}
$s='#include <Native/HeapPlacement.hpp>'+[Environment]::NewLine+'#include "../native_save_envelope.hpp"'+[Environment]::NewLine+$s
$s += Get-Content "$n\src\save_handler_boundary.inc" -Raw
Set-Content "$n\src\generated\SaveDataHandler.cpp" $s
