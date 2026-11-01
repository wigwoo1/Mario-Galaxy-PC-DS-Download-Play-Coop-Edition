$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
New-Item -ItemType Directory -Force -Path (Join-Path $n 'build\flag-arrays') | Out-Null
& (Join-Path $n 'generate-flag-vertex-arrays.ps1')
$generated=[IO.File]::ReadAllText((Join-Path $n 'src\generated\Flag.cpp'))
$loadMaterial='void Flag::loadMaterial() const {'+"`n"+'    Native::refreshFlagVertexArrays(mColors,mTexST,mNumPointsV,mNumPointsU);'
if(!$generated.Replace("`r`n","`n").Contains($loadMaterial)){throw 'Generated Flag::loadMaterial must refresh the arrays before binding'}
$original=[IO.File]::ReadAllText((Join-Path $p 'Petari\src\Game\Map\Flag.cpp'))
$begin=$original.IndexOf('    mColors = new Color8[');$end=$original.IndexOf('    if (mObjName != nullptr)', $begin)
if($begin-lt 0-or $end-lt $begin){throw 'Original flag array initialization missing'}
$init=$original.Substring($begin,$end-$begin).Replace('new Color8[mNumPointsV * (mNumPointsU + 1)]','Native::allocateFlagColors(mNumPointsV,mNumPointsU)').Replace('new f32[mNumPointsV * (mNumPointsU + 1) * 2]','Native::allocateFlagTexCoords(mNumPointsV,mNumPointsU)')
$binding="    GXSetArray(GX_VA_CLR0, mColors, 4);`n    GXSetArray(GX_VA_TEX0, mTexST, 8);"
if(!$original.Replace("`r`n","`n").Contains($binding)){throw 'Original flag array binding changed'}
$fixture="struct OriginalFlagArrayFixture { s32 mNumPointsV,mNumPointsU; Color8* mColors=nullptr; f32* mTexST=nullptr; void init(){`n"+$init+"}`nvoid bind(){Native::refreshFlagVertexArrays(mColors,mTexST,mNumPointsV,mNumPointsU);`n"+$binding+"`n}};"
[IO.File]::WriteAllText((Join-Path $n 'build\flag-arrays\original-array-fixture.inc'),$fixture,[Text.UTF8Encoding]::new($false))
$env:ZIG_GLOBAL_CACHE_DIR=Join-Path $n 'build\zig-cache-player-display-lists'
$env:ZIG_LOCAL_CACHE_DIR=Join-Path $n 'build\zig-local-cache'
$exe=Join-Path $n 'build\flag-array-tests.exe'
$log=Join-Path $n 'build\flag-array-tests.log'
$runLog=Join-Path $n 'build\flag-array-tests-run.log'
$line=Get-Content -LiteralPath (Join-Path $n 'test-unit-heap-live.ps1') | Where-Object {$_ -like '& *zig.exe*'}
if(@($line).Count-ne 1){throw 'Expected one real heap regression compiler command'}
$line=$line.Replace('unit_heap_live_tests.cpp','flag_array_tests.cpp').Replace(' -std=c++17',' -std=c++17 -DGALAXY_ORIGINAL_GD_GEOMETRY -I"'+(Join-Path $n 'build\flag-arrays')+'"')
$providers=' "$n\src\gpu_resources.cpp" "$n\src\gd_storage.cpp" "$n\src\gx_array_port.cpp" "$n\src\sdk_cache_port.cpp" "$n\src\generated\GDGeometry.cpp" '
$line=$line.Replace(' -o $exe',$providers+' -o $exe')
Invoke-Expression $line
if($LASTEXITCODE){Get-Content -LiteralPath $log -Tail 30;exit 1}
& $exe *> $runLog
$result=$LASTEXITCODE
Get-Content -LiteralPath $runLog
exit $result
