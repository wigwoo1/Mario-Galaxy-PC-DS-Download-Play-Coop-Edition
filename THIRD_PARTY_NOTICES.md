# Source provenance

Petari/ is based on [SMGCommunity/Petari](https://github.com/SMGCommunity/Petari), revision 0d9b80c569b954562c80206113e6936378c90c27, with local port edits. Preserve Petari/LICENSE (CC0 1.0 Universal) and individual-file notices. Existing upstream source constants are retained; extracted retail directories are excluded. The locally modified Petari source is included, not replaced by a pristine upstream checkout.

This publication does not introduce a new blanket license for native additions or all third-party material. No ownership of Nintendo trademarks or game data is claimed.

Zig is downloaded separately using tools/zig-package.json; its official distribution supplies compiler/runtime notices. Some DSP reference scripts refer to a separately fetched [Dolphin](https://github.com/dolphin-emu/dolphin) checkout, revision e2ad09306e6d371f7272205bdc363da9fe05fe08. That checkout is distinct from the newly vendored renderer copies.

The Zelda audio renderer imports and native adaptations are **GPL-2.0-or-later**, copyright Dolphin Emulator Project. [native/third_party/dolphin-zelda](native/third_party/dolphin-zelda) contains the pristine Zelda.cpp/Zelda.h copies, UPSTREAM-COPYING, GPL-2.0.txt, source hashes and [adaptation/provenance notice](native/third_party/dolphin-zelda/NOTICE.txt). The imported local reference copies have exact content hashes; no upstream Git revision is claimed for those copies. The generated renderer, frame executor, adaptation generator and related derivative source are included. Their GPL designation is separate from Petari's CC0 notice.

The unchanged October 27 diagnostic ZIP predates this renderer integration. No newer renderer-containing binary is redistributed in this publication. Preserve the corresponding source and license notices when distributing builds that incorporate it.

Public source availability does not authorize redistribution of separately supplied game assets.
