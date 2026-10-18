param([switch]$StageTables)
$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$sourceUnit=if($StageTables){'StageDataHolder.cpp'}else{'ObjUtil.cpp'}
$source=Get-Content -LiteralPath (Join-Path $n ('src\generated\'+$sourceUnit)) -Raw
$signature=if($StageTables){'static void attachStageArchiveTable('}else{'JMapInfo* tryCreateCsvParserLocal('}
$begin=$source.IndexOf($signature)
$open=$source.IndexOf('{',$begin);$depth=1;$end=$open+1
if($begin-lt 0-or $open-lt $begin){throw 'CSV factory helper anchor missing.'}
while($depth-and $end-lt $source.Length){if($source[$end]-eq '{'){$depth++};if($source[$end]-eq '}'){$depth--};++$end}
if($depth){throw 'CSV helper body is not balanced.'}
$fixtureName=if($StageTables){'stage_resource_attachment_tests.cpp'}else{'csv_resource_tests.cpp'}
$fixture=Get-Content -LiteralPath (Join-Path $n ('src\'+$fixtureName)) -Raw
$fixture=$fixture.Replace('// ACTUAL_CSV_FACTORY_INSERTION_POINT',$source.Substring($begin,$end-$begin))
if($StageTables){
 $ownerBegin=$source.IndexOf('const StageDataHolder* StageDataHolder::findPlacedStageDataHolder(')
 $ownerOpen=$source.IndexOf('{',$ownerBegin);$ownerEnd=$ownerOpen+1;$ownerDepth=1
 if($ownerBegin-lt 0){throw 'Stage ownership method anchor missing.'}
 while($ownerDepth-and $ownerEnd-lt $source.Length){if($source[$ownerEnd]-eq '{'){$ownerDepth++};if($source[$ownerEnd]-eq '}'){$ownerDepth--};++$ownerEnd}
 if($ownerDepth){throw 'Stage ownership method is not balanced.'}
 $fixture=$fixture.Replace('// ACTUAL_STAGE_OWNERSHIP_INSERTION_POINT',$source.Substring($ownerBegin,$ownerEnd-$ownerBegin))
}
$outputName=if($StageTables){'stage-resource-attachment-test'}else{'csv-resource-test'}
$out=Join-Path $n ('build\'+$outputName);New-Item -ItemType Directory -Force -Path $out|Out-Null
$cpp=Join-Path $out 'csv_resource_tests.cpp';$exe=Join-Path $out 'csv-resource-tests.exe'
[IO.File]::WriteAllText($cpp,$fixture,[Text.UTF8Encoding]::new($false))
$env:ZIG_GLOBAL_CACHE_DIR=Join-Path $p 'tools\zig-cache'
$compileArgs=@('-std=c++17','-O2','-UNDEBUG','-ffunction-sections','-Wl,--gc-sections','-target','x86_64-windows-gnu')
foreach($relative in @('native\src','native\include-runtime','native\include','Petari\include','Petari\libs\JSystem\include','Petari\libs\RVL_SDK\include')){$compileArgs+=('-I'+(Join-Path $p $relative))}
$compileArgs+=@('-include','revolution/types.h',$cpp,(Join-Path $n 'src\generated\JMapInfo.cpp'),(Join-Path $p 'Petari\src\JSystem\JGadget\hashcode.cpp'),(Join-Path $n 'src\string_equal_case_port.cpp'),'-lbcrypt','-o',$exe)
& (Join-Path $p 'tools\zig-x86_64-windows-0.16.0\zig.exe') c++ @compileArgs *> (Join-Path $out 'build.log')
if($LASTEXITCODE){Get-Content -LiteralPath (Join-Path $out 'build.log') -Tail 22;exit $LASTEXITCODE}
& $exe 'D:\Desktop\Super Mario Galaxy (USA, Canada) (En,Fr,Es).wbfs'
exit $LASTEXITCODE
