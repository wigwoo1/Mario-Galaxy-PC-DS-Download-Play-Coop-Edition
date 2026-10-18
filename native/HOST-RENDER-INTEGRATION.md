# Native host render integration

## Implemented and verified (2026-10-12)

`NativeD3dGameHost` owns a Win32 window, a hardware D3D11 device with WARP fallback, the GX EFB, a persistent GX command-submit binding, and a D3D11/DXGI XFB presenter. The generated `GameSystem::init` constructs this singleton after `GameSystemObjHolder::init` creates the video mode and three XFBs, but before the first game frame. `GameSystem::frameLoop` pumps its window messages, registers the current JUTXfb buffers in the same GPU resource store, and scopes display-copy observation to each frame. The submitter sends completed display-copy bytes to the presenter's YUYV-to-RGB conversion and swap-chain path.

`test-d3d-game-host.ps1` passes synthetic GX display-copy execution through the complete host on WARP. It checks the EFB-to-XFB output and exercises the DXGI presentation callback. `test-d3d-xfb-present.ps1` independently verifies known color values in the D3D backbuffer before calling `Present`. The generated `GameSystem.cpp` compiles with the host integration, and `generate-native-game-system.ps1 -OutputPath <temporary-file>` reproduces its contents.

## Still unverified

Source tracing confirms the retail setup selects three XFBs; its first triple-buffer display copy happens from `MainLoopFramework::endRender`, after the host is initialized. The synthetic test verifies that copy path, but this does not establish a successful full Petari link or retail startup. Input binding to the created HWND, clean window-close behavior, native WBFS boot, and visible retail gameplay remain unverified. There is no verified playable game yet; overall end-to-end project completion remains **0%**.
