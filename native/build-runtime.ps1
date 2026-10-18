$ErrorActionPreference='Stop'
. "$PSScriptRoot\test-original-collision-scene.ps1"
if($LASTEXITCODE){exit 1}
$line=$line.Replace('c++ -DNATIVE_COLLISION_SCENE','c++ -shared -DNATIVE_COLLISION_SCENE').Replace('src\original_kcl_server_tests.cpp','src\native_runtime_api.cpp').Replace('original-collision-scene-tests.exe','galaxy-runtime.dll').Replace('original-collision-scene-tests.log','galaxy-runtime-build.log')
Invoke-Expression $line
if($LASTEXITCODE){Get-Content "$n\build\galaxy-runtime-build.log" -Tail 25;exit 1}
Write-Output "Built $n\build\galaxy-runtime.dll"
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -O2 -UNDEBUG -target x86_64-windows-gnu "$n\src\native_runtime_dll_tests.cpp" -o "$n\build\native-runtime-dll-tests.exe"
if($LASTEXITCODE){exit 1}
foreach($asset in @('broadbeanplanet','broadbeanplanetc')){& "$n\build\native-runtime-dll-tests.exe" "$n\build\galaxy-runtime.dll" "$n\build\sample-assets\$asset.kcl" "$n\build\sample-assets\$asset.pa";if($LASTEXITCODE){exit 1}}