$ErrorActionPreference='Stop'
$project='D:\Desktop\Games\Mario galaxy'
$native=Join-Path $project 'native'
$output=Join-Path $PSScriptRoot 'build'
New-Item -ItemType Directory -Force -Path $output | Out-Null
$env:ZIG_GLOBAL_CACHE_DIR=Join-Path $output 'zig-cache'
& "$project\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -O2 -UNDEBUG -target x86_64-windows-gnu "-I$native\src" "-I$native\include-runtime" "-I$native\include" "-I$project\Petari\include" "-I$project\Petari\libs\JSystem\include" "-I$project\Petari\libs\nw4r\include" "-I$project\Petari\libs\RVL_SDK\include" -include revolution/types.h -include "$native\include-runtime\nw4r\ut\TagProcessorBase.h" "$native\src\message_record_tests.cpp" "$native\src\generated\MessageTagAccessKernel.cpp" "$native\src\generated\MessageRecordKernel.cpp" "$native\src\generated\TalkNodeNavigationKernel.cpp" "$native\src\generated\MessageHolderLifecycleKernel.cpp" "$native\src\game_message_backend.cpp" "$native\src\generated\JMapInfo.cpp" "$native\src\string_equal_case_port.cpp" "$native\src\sensor_hash_port.cpp" "$project\Petari\src\JSystem\JGadget\hashcode.cpp" "$project\Petari\src\Game\NPC\TalkMessageInfo.cpp" "$project\Petari\src\nw4r\ut\ut_Font.cpp" "$project\Petari\src\nw4r\ut\ut_ResFontBase.cpp" "$project\Petari\src\nw4r\ut\ut_CharStrmReader.cpp" -lbcrypt -o "$output\message-record-tests.exe" *> "$output\message-record-tests.log"
if($LASTEXITCODE){Get-Content "$output\message-record-tests.log" -Tail 15;exit 1}
& "$output\message-record-tests.exe" "D:\Desktop\Super Mario Galaxy (USA, Canada) (En,Fr,Es).wbfs"
exit $LASTEXITCODE











