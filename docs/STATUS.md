# Current status — 2026-10-27 publication

**Not playable.** The latest existing diagnostic build selects saved Mario slot 1, accepts Start, loads profile data and progresses past Kinopio's former BRK-start failure. Stage placement then crashes in Butterfly initialization while an original color animation writes through missing material storage. No completed stage frame, player-controlled gameplay, audible game audio or campaign playthrough is verified.

Current source includes a subsequent material-storage repair. Its focused retail fixture passes two animation attachments, 18 register checks and nine Mario material GPU checks. The published runtime predates this final source repair; a broader regression, full rebuild and another stage run are still required. Component success does not establish a repaired full game.

Earlier completed checks cover native BRK conversion, original samplers and material matching, shader reuse with 187 reference GPU comparisons, both 16-stage TEV paths and existing model-rendering regressions. Actual original save verification passed with unchanged hashes. No game assets or saves are published.

The standalone probe remains unchanged; its publication self-test last passed 2026-10-18. The ZIP contains a hash-verified 2026-10-27 launcher/DLL pair and the unchanged probe. See [build manifest](../builds/manifest.json) and [verification](verification/2026-10-27-native-color-animation.md).

Rough whole-project estimate: about **3%**, not a measured counter. Verified playable progress: **0%**. Settings, HD integration and multiplayer remain future work.
