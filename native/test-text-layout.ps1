$ErrorActionPreference='Stop'
$project='D:\Desktop\Games\Mario galaxy'
$native=Join-Path $project 'native'
# Test-only template selection excludes unused wide instantiations; bodies remain original.
foreach($pair in @(@('ut_TextWriterBase','NarrowTextWriterBaseTest','TextWriterBase'),@('ut_TagProcessorBase','NarrowTagProcessorTest','TagProcessorBase'))){
$source=Get-Content "$project\Petari\src\nw4r\ut\$($pair[0]).cpp" -Raw
$source=$source.Replace("        template class $($pair[2])< wchar_t >;",'')
Set-Content "$native\src\generated\$($pair[1]).cpp" $source
}
$output=Join-Path $PSScriptRoot 'build'
New-Item -ItemType Directory -Force -Path $output | Out-Null
$env:ZIG_GLOBAL_CACHE_DIR=Join-Path $output 'zig-cache'
& "$project\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -O2 -UNDEBUG -ffunction-sections -fdata-sections "-Wl,--gc-sections" -target x86_64-windows-gnu "-I$native\src" "-I$native\include-runtime" "-I$native\include" "-I$project\Petari\libs\nw4r\include" "-I$project\Petari\libs\RVL_SDK\include" -include revolution/types.h -include "$native\include-runtime\nw4r\ut\TagProcessorBase.h" "$native\src\text_layout_tests.cpp" "$native\src\resource_font_port.cpp" "$native\src\gpu_resources.cpp" "$project\Petari\src\nw4r\ut\ut_Font.cpp" "$project\Petari\src\nw4r\ut\ut_ResFontBase.cpp" "$project\Petari\src\nw4r\ut\ut_CharStrmReader.cpp" "$native\src\generated\NarrowTagProcessorTest.cpp" "$native\src\generated\NarrowTextWriterBaseTest.cpp" "$native\src\generated\NarrowTextWriter.cpp" -o "$output\text-layout-tests.exe" *> "$output\text-layout-tests.log"
if($LASTEXITCODE){Get-Content "$output\text-layout-tests.log" -Tail 15;exit 1}
& "$output\text-layout-tests.exe" "$native\build\menufont64-reference.brfnt"
exit $LASTEXITCODE








