& "$PSScriptRoot\generate-texture-pattern-sampler.ps1"
& "$PSScriptRoot\generate-visibility-sampler.ps1"
$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent;$env:ZIG_GLOBAL_CACHE_DIR="$p\tools\zig-cache"
& "$n\generate-jmath-inline.ps1"
& "$n\generate-animation-sampler.ps1"
& "$n\generate-native-speaker-data.ps1"
& "$n\generate-native-speaker-wave.ps1"
$line=Get-Content "$n\test-unit-heap-live.ps1" | Where-Object {$_ -like '& *zig.exe*'}
$providers=' "$n\src\aram_read_port.cpp" "$n\src\native_jkr_archive_port.cpp" "$n\src\generated\JKRCompArchive.cpp" "$n\src\generated\JKRAramArchive.cpp" "$n\src\aram_manager_kernel.cpp" "$n\src\generated\JKRAramHeap.cpp" "$n\src\generated\JKRAramBlock.cpp" "$n\src\native_dvd_port.cpp" "$n\src\native_jkr_aram_port.cpp" "$n\src\generated\AramCompletionKernel.cpp" "$n\src\generated\JKRDecomp.cpp" "$n\src\generated\JKRThread.cpp" "$n\src\jut_console_port.cpp" "$n\src\sdk_cache_port.cpp" "$n\src\gpu_resources.cpp" "$n\src\generated\JKRMemArchive.cpp" "$n\src\generated\JKRDvdArchive.cpp" "$n\src\generated\HeapCopyKernel.cpp" "$n\src\generated\JKRDvdFile.cpp" "$n\src\generated\JKRDvdRipper.cpp" "$n\src\generated\JKRArchivePub.cpp" "$p\Petari\src\JSystem\JKernel\JKRArchivePri.cpp" "$p\Petari\src\JSystem\JKernel\JKRFileLoader.cpp" "$p\Petari\src\JSystem\JKernel\JKRFileFinder.cpp" '
$providers+= ' "$n\src\native_bck_animation.cpp" "$n\src\generated\OriginalTransformSampler.cpp" "$n\src\generated\VisibilitySampler.cpp" "$n\src\generated\TexturePatternSampler.cpp" "$p\Petari\src\JSystem\JUtility\JUTNameTab.cpp" '
$providers+= ' "-I$n\build\camera-pointer-candidate\include" "$n\src\generated\SpkData.cpp" "$n\src\generated\SpkTableConstructor.cpp" "$n\src\generated\SpeakerWaveGettersFixture.cpp" '
$line=$line.Replace('src\unit_heap_live_tests.cpp','src\original_archive_mount_tests.cpp').Replace(' -o ',$providers+' -lbcrypt -o ').Replace('unit-heap-live-tests','original-archive-mount-tests')
$line=$line.Replace(' "-I$n\include-runtime"',' "-I$n\build\camera-pointer-candidate\include" "-I$n\include-runtime"')
Invoke-Expression $line
if($LASTEXITCODE){Get-Content "$n\build\original-archive-mount-tests.log" -Tail 35;exit 1}
& "$n\build\original-archive-mount-tests.exe" 'D:\Desktop\Super Mario Galaxy (USA, Canada) (En,Fr,Es).wbfs'
if($LASTEXITCODE){if($LASTEXITCODE){exit $LASTEXITCODE}
& "$n\build\original-archive-mount-tests.exe" 'D:\Desktop\Super Mario Galaxy (USA, Canada) (En,Fr,Es).wbfs' '/ObjectData/MarioAnime.arc'
exit $LASTEXITCODE}
& "$n\build\original-archive-mount-tests.exe" 'D:\Desktop\Super Mario Galaxy (USA, Canada) (En,Fr,Es).wbfs' '/ObjectData/Mario.arc'
if($LASTEXITCODE){exit $LASTEXITCODE}
& "$n\build\original-archive-mount-tests.exe" 'D:\Desktop\Super Mario Galaxy (USA, Canada) (En,Fr,Es).wbfs' '/ObjectData/MarioAnime.arc'
if($LASTEXITCODE){exit $LASTEXITCODE}
& "$n\build\original-archive-mount-tests.exe" 'D:\Desktop\Super Mario Galaxy (USA, Canada) (En,Fr,Es).wbfs' '/ObjectData/BeeMarioWing.arc'
if($LASTEXITCODE){exit $LASTEXITCODE}
& "$n\build\original-archive-mount-tests.exe" 'D:\Desktop\Super Mario Galaxy (USA, Canada) (En,Fr,Es).wbfs' '/ObjectData/BeeMario.arc'
if($LASTEXITCODE){exit $LASTEXITCODE}
& "$n\build\original-archive-mount-tests.exe" 'D:\Desktop\Super Mario Galaxy (USA, Canada) (En,Fr,Es).wbfs' '/AudioRes/SpkRes/SpkRes.arc'
exit $LASTEXITCODE
















