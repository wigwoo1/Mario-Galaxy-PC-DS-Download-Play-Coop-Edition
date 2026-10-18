$ErrorActionPreference='Stop'
$n=$PSScriptRoot
$p=Split-Path $n -Parent
$out="$n\build\NativeRetailTables.cpp"
$tool="$n\build\generate-native-retail-tables.exe"
$env:ZIG_GLOBAL_CACHE_DIR="$p\tools\zig-cache"
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -O2 "-I$n\src" "$n\src\generate_native_retail_tables.cpp" -o $tool *> "$n\build\generate-native-retail-tables.log"
if($LASTEXITCODE){Get-Content "$n\build\generate-native-retail-tables.log" -Tail 15;exit 1}
& $tool "$p\Petari\src\Game\System\StoryEvent.bcsv" "$p\Petari\src\Game\System\GalaxyID.bcsv" $out
exit $LASTEXITCODE