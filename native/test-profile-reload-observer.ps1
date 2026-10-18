# Compile the read-only observer in the full original FileSelector. This creates
# an isolated object only and never replaces a live game DLL or modifies saves.
param([string]$SourcePath)
$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
if(!$SourcePath){$SourcePath=Join-Path $n 'build\startup-recovery-temp\galaxy-dspboot-link-verification\FileSelector-native-font.cpp'}
$source=Get-Content -LiteralPath $SourcePath -Raw
$observer=Get-Content -LiteralPath "$n\src\diagnostic_profile_reload.inc" -Raw
if(!$source.Contains('// Native profile reload observer.')){
 $signature='void FileSelector::exeFileSelect() {'
 if([regex]::Matches($source,[regex]::Escape($signature)).Count-ne 1){throw 'Original menu function anchor missing or duplicated'}
 $source=$source.Replace($signature,$signature+"`n"+$observer)
}
$build=Join-Path $n 'build\profile-reload-observer';New-Item -ItemType Directory -Force -Path $build | Out-Null
$file=Join-Path $build 'FileSelector-reload-observed.cpp';[IO.File]::WriteAllText($file,$source)
$env:ZIG_LOCAL_CACHE_DIR=Join-Path $build 'cache'
$compileArgs=@('-DGALAXY_NATIVE_JAI_SOUND_ID=1','-DGALAXY_NATIVE_JPA_USERWORK_64=1','-DGALAXY_ORIGINAL_MOVEMENT_CONTROLLER','-std=c++17','-O2','-c','-target','x86_64-windows-gnu')
foreach($path in @('native\include-runtime','native\include','Petari\include','Petari\libs\JSystem\include','Petari\libs\RVL_SDK\include','Petari\libs\RVLFaceLib\include','Petari\libs\nw4r\include')){$compileArgs+=('-I'+(Join-Path $p $path))}
$compileArgs+=@('-include','revolution/types.h','-include',"$n\include-runtime\nw4r\ut\TagProcessorBase.h",'-include',"$n\include-runtime\nw4r\ut\Color.h")
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ @compileArgs $file '-o' "$build\FileSelector-reload-observed.obj" *> "$build\compile.log"
if($LASTEXITCODE){Get-Content "$build\compile.log" -Tail 20;exit $LASTEXITCODE}
Write-Output 'PASS read-only reload observer compiled in full original FileSelector; no game or save data modified'
