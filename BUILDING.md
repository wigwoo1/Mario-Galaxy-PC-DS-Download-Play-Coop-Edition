# Building diagnostics

Requires Windows x64, PowerShell, Git and Zig matching tools/zig-package.json. The compiler itself is not vendored. The local layout expects tools/zig-x86_64-windows-0.16.0/zig.exe.

From the checkout, run:

    powershell -ExecutionPolicy Bypass -File publishing/prepare-build.ps1
    powershell -ExecutionPolicy Bypass -File native/build.ps1

The setup helper downloads the official pinned Zig archive and checks its SHA-256, prepares diagnostic support and fetches a pinned Dolphin reference. The helper has been reviewed but not tested on a clean computer. native/build.ps1 builds the probe and runs self-tests plus DSP component checks. Other native/test-*.ps1 scripts check narrower components; some need your own game image or generated resources.

## Full boot diagnostic

The published launcher/DLL pair linked and ran locally on 2026-10-27. Rebuilding the complete boot diagnostic currently involves native/probe-actor-link.ps1, generated source providers and native/test-actor-runtime-dsp-boot-link.ps1. Historical scripts use generated object/response caches and some local paths. Caches and extracted data are excluded. A clean-checkout, one-command full boot rebuild has not been verified. See docs/verification/2026-10-27-native-color-animation.md for the current crash and builds/manifest.json for exact artifact hashes.

Supply game data locally for disc-backed checks. Never commit images, extracted archives/models/textures, saves, dumps or texture packs. Existing upstream source constants remain under their notices; this snapshot is not an extracted retail filesystem.
