$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$root=Split-Path $n -Parent
$path=Join-Path $root 'Petari\src\Game\Scene\SceneObjHolder.cpp'
$source=[IO.File]::ReadAllText($path)
$signature='NameObj* SceneObjHolder::newEachObj(int id) {'
if([regex]::Matches($source,[regex]::Escape($signature)).Count-ne 1){throw 'Expected one original scene factory.'}
$start=$source.IndexOf($signature)
$open=$source.IndexOf('{',$start);$depth=1;$end=$open+1
while($depth -and $end-lt $source.Length){if($source[$end]-eq '{'){$depth++};if($source[$end]-eq '}'){$depth--};$end++}
if($depth){throw 'Unterminated original scene factory.'}
$includeEnd=$source.IndexOf('SceneObjHolder::SceneObjHolder()')
if($includeEnd-lt 0-or $includeEnd-ge $start){throw 'Original factory include boundary missing.'}
$body=$source.Substring($start,$end-$start)
$cases=[regex]::Matches($body,'case (SceneObj_\w+):')
if($cases.Count-lt 1 -or ($cases|ForEach-Object {$_.Groups[1].Value}|Select-Object -Unique).Count-ne $cases.Count){throw 'Missing or duplicate original factory cases.'}
$renamed=$body.Replace($signature,'inline NameObj* nativeOriginalSceneObject(int id) {')
$generated='#pragma once'+"`n"+'// Generated from Petari SceneObjHolder.cpp; original constructor selection is retained.'+"`n"+$source.Substring(0,$includeEnd)+$renamed+"`n"
$out=Join-Path $n 'src\generated\OriginalSceneObjFactory.hpp'
[IO.File]::WriteAllText($out,$generated,[Text.UTF8Encoding]::new($false))
Write-Output ('Generated original scene factory: '+$cases.Count+' authored constructor cases.')