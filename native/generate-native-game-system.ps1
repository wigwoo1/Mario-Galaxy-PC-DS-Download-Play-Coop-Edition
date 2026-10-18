param([string]$OutputPath)
$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\System\GameSystem.cpp" -Raw
if(!$s.Contains('void main(void)')){throw 'Original game entry anchor missing'}
$s=$s.Replace('void main(void)','extern "C" void nativeOriginalGameMain(void)')
$s=$s.Replace('#include "Game/System/GameSystem.hpp"','#include "Game/System/GameSystem.hpp"' + "`n" + '#include <Native/GameInput.hpp>')
$s=$s.Replace('#include <Native/GameInput.hpp>','#include <Native/GameInput.hpp>' + "`n" + '#include <Native/DisplayCopyBinding.hpp>'+"
"+'#include <Native/TextureCopyBinding.hpp>' + "`n" + '#include <Native/GxCommandState.hpp>' + "`n" + '#include <Native/GpuResources.hpp>' + "`n" + '#include <Native/XfbResourceRegistry.hpp>' + "`n" + '#include "native_d3d_game_host.hpp"')
$s=$s.Replace('#include <JSystem/JKernel/JKRExpHeap.hpp>','#include <JSystem/JKernel/JKRExpHeap.hpp>' + "`n" + '#include <JSystem/JUtility/JUTVideo.hpp>' + "`n" + '#include <JSystem/JUtility/JUTXfb.hpp>')
if(!$s.Contains('    mObjHolder->init();')){throw 'GameSystem video initialization anchor missing'}
$s=$s.Replace('    mObjHolder->init();','    mObjHolder->init();' + "`n" + '    auto* nativeVideoManager = JUTVideo::getManager();' + "`n" + '    if (!nativeVideoManager || !nativeVideoManager->getRenderMode())' + "`n" + '        throw std::logic_error("Native game host requires an initialized video mode");' + "`n" + '    const auto* nativeMode = nativeVideoManager->getRenderMode();' + "`n" + '    NativeD3dGameHost::initialize(nativeMode->fbWidth, nativeMode->efbHeight);')
if(!$s.Contains('void GameSystem::frameLoop() {')){throw 'GameSystem frame-loop input anchor missing'}
$frame='void GameSystem::frameLoop() {' + "`n" + '    static Native::XfbResourceRegistry xfbResources;' + "`n" + '    auto* xfbManager = JUTXfb::getManager();' + "`n" + '    auto* videoManager = JUTVideo::getManager();' + "`n" + '    if (!xfbManager || !videoManager || !videoManager->getRenderMode())' + "`n" + '        throw std::logic_error("Native game frame loop requires initialized video/XFB managers");' + "`n" + '    const auto* mode = videoManager->getRenderMode();' + "`n" + '    xfbResources.refresh(Native::gpuResources(),' + "`n" + '        {xfbManager->mXfbs[0], xfbManager->mXfbs[1], xfbManager->mXfbs[2]},' + "`n" + '        static_cast<unsigned>(xfbManager->getBufferNum()), mode->fbWidth, mode->xfbHeight);' + "`n" + '    static Native::GxCommandState displayCopyState;' + "`n" + '    Native::DisplayCopyBinding displayCopyBinding(displayCopyState, Native::gpuResources());' + "`n" + '    nativePumpGameInputFrame();'
$frame=$frame.Replace('    xfbResources.refresh(Native::gpuResources(),', '    auto& renderHost = NativeD3dGameHost::instance();' + "`n" + '    if (!renderHost.pumpMessages()) throw std::runtime_error("Native game window closed");' + "`n" + '    xfbResources.refresh(renderHost.resources(),')
$frame=$frame.Replace('Native::DisplayCopyBinding displayCopyBinding(displayCopyState, Native::gpuResources());','Native::DisplayCopyBinding displayCopyBinding(displayCopyState, renderHost.resources());')
$frame=$frame.Replace('    Native::DisplayCopyBinding displayCopyBinding(displayCopyState, renderHost.resources());','    Native::DisplayCopyBinding displayCopyBinding(displayCopyState, renderHost.resources());'+"`n"+'    Native::TextureCopyBinding textureCopyBinding(displayCopyState, renderHost.resources());')
$s=$s.Replace('void GameSystem::frameLoop() {',$frame)
if(!$OutputPath){$OutputPath="$n\src\generated\GameSystem.cpp"}
[IO.File]::WriteAllText([IO.Path]::GetFullPath($OutputPath),$s,[Text.UTF8Encoding]::new($false))
