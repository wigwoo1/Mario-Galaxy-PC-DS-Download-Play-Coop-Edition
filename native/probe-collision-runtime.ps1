# Compile original collision runtime using the full original actor layout; never link against diagnostic layout.
$ErrorActionPreference='Stop'
$p=Split-Path $PSScriptRoot -Parent
$out=Join-Path $PSScriptRoot 'build\collision-runtime-probe'
New-Item -ItemType Directory -Force $out | Out-Null
$env:ZIG_GLOBAL_CACHE_DIR=Join-Path $p 'tools\zig-cache'
$paths=@('NameObj\NameObj','NameObj\NameObjHolder','NameObj\NameObjRegister','LiveActor\Binder','Map\HitInfo','Map\CollisionParts','Map\CollisionCode','Map\CollisionCategorizedKeeper','Map\CollisionDirector','Map\KCollision','Map\KCollisionPlus','Util\CollisionPartsFilter')
& (Join-Path $PSScriptRoot 'generate-collision-runtime.ps1')
$holderSource=[IO.File]::ReadAllText("$p\Petari\src\Game\NameObj\NameObjHolder.cpp")
$old='std::for_each(mObjArray1.begin(), mObjArray1.end(), std::mem_fun(pMethod));'
if(!$holderSource.Contains($old)){throw 'Original holder member dispatch anchor missing'}
$holderSource=$holderSource.Replace($old,'std::for_each(mObjArray1.begin(), mObjArray1.end(), [pMethod](NameObj* object) { (object->*pMethod)(); });')
[IO.File]::WriteAllText("$PSScriptRoot\src\generated\NameObjHolder.cpp",$holderSource)
$results=@()
foreach($path in $paths){
 $unit=Split-Path $path -Leaf
 $source=Join-Path $p "Petari\src\Game\$path.cpp"
 if($unit -in @('KCollision','CollisionParts','NameObjHolder')){$source=Join-Path $PSScriptRoot "src\generated\$unit.cpp"}
 & "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -c -ferror-limit=20 -target x86_64-windows-gnu "-I$PSScriptRoot\include-runtime" "-I$PSScriptRoot\include" "-I$p\Petari\include" "-I$p\Petari\libs\JSystem\include" "-I$p\Petari\libs\RVL_SDK\include" "-I$p\Petari\libs\nw4r\include" -include revolution/types.h $source -o "$out\$unit.obj" *> "$out\$unit.log"
 $results += [pscustomobject]@{Unit=$unit;ExitCode=$LASTEXITCODE;Log="$out\$unit.log"}
}
$results | ConvertTo-Json | Set-Content "$out\results.json"
$results | Format-Table -AutoSize
if(@($results|Where-Object ExitCode -ne 0).Count){exit 1}
