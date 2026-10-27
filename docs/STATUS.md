# Current status — 2026-10-27 publication

**Not playable.** The existing diagnostic build selects saved Mario slot 1, accepts Start, loads profile data, completes placement and reaches SCENE READY. It progresses beyond the earlier Butterfly/material and silhouette/matrix faults, then fails in normal Mario drawing when GXCallDisplayList receives an unregistered private display-list buffer. No complete stage frame, player-controlled gameplay, audible game audio or campaign playthrough is verified.

Current source includes the original scene factory fallback and extra-matrix ownership repair. Recorded development checks pass the full diagnostic link, original model regression and eight-view retail MarioShadow matrix fixture. The factory preserves the original 118-case body and 61 existing native routes. Component success does not establish a working full game. Actual save verification passed with unchanged hashes; no assets or saves are published.

The ZIP contains the hash-verified 2026-10-27T12:13:04.6700080-04:00 launcher/DLL pair and unchanged standalone probe (last self-test PASS 2026-10-18). The publisher did not compile these binaries. The runtime has automated diagnostic input and a hidden window. See the [build manifest](../builds/manifest.json) and [verification summary](verification/2026-10-27-scene-factory-extra-matrices.md).

Rough whole-project estimate: about **3%**, not a measured counter. Verified playable progress: **0%**. Settings, HD integration and multiplayer remain future work.
