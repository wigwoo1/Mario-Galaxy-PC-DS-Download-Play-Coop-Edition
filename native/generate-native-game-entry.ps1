$ErrorActionPreference = 'Stop'
$native = $PSScriptRoot
$project = Split-Path $native -Parent
$source = Get-Content -LiteralPath "$project\Petari\src\Game\System\GameSystem.cpp" -Raw
$source = $source.Replace('void main(void)', 'extern "C" __declspec(dllexport) void nativeOriginalGameMain(void)')
$anchor = 'extern "C" __declspec(dllexport) void nativeOriginalGameMain(void)'
$start = $source.IndexOf($anchor, [StringComparison]::Ordinal)
if ($start -lt 0) { throw 'Could not find Petari game entry point' }
$open = $source.IndexOf('{', $start)
if ($open -lt 0) { throw 'Could not find Petari game entry body' }
$depth = 0
$end = -1
for ($i = $open; $i -lt $source.Length; $i++) {
    if ($source[$i] -eq '{') { $depth++ }
    elseif ($source[$i] -eq '}') {
        $depth--
        if ($depth -eq 0) { $end = $i + 1; break }
    }
}
if ($end -lt 0) { throw 'Petari game entry body is unterminated' }
$entry = $source.Substring($start, $end - $start)
$entry = $entry.Replace($anchor + ' {', $anchor + " {`n    Native::registerExecutableImageResources(reinterpret_cast<const void*>(&nativeOriginalGameMain));")
$prefix = @'
#include "Game/System/GameSystem.hpp"
#include "Game/System/HeapMemoryWatcher.hpp"
#include "Game/System/GameSystemException.hpp"
#include "Game/System/FileRipper.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/MemoryUtil.hpp"
#include "Game/Util/MutexHolder.hpp"
#include "Game/Util/SingletonHolder.hpp"
#include "Game/Util/SystemUtil.hpp"
#include <JSystem/JKernel/JKRAram.hpp>
#include <JSystem/JKernel/JKRExpHeap.hpp>
#include <nw4r/lyt/init.h>
#include <revolution.h>
#include <Native/GpuResources.hpp>

'@
$output = Join-Path $native 'src\generated\GameSystemEntry.cpp'
Set-Content -LiteralPath $output -Value ($prefix + $entry + "`n") -Encoding utf8
Write-Output "Generated $output from Petari GameSystem::main entry body"
