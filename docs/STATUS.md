# Current status — November 1, 2026 publication

**Not playable. Development remains paused by the user.**

Source has advanced beyond the October 27 packaged diagnostics. Player display-list ownership and flag arrays received repairs, and a development run captured a partial Star Festival stage view with Mario before failing during original camera-animation evaluation. No complete frame marker, reliable player control, campaign playthrough or full-fidelity rendering is verified. Camera decoding/ownership now has focused retail-asset tests; a later full-game camera retest did not reach that camera and does not prove the repair in gameplay.

The native audio path now includes a GPL-2.0-or-later Dolphin-derived renderer, actual JAS/ARAM parameter translation, deferred SDK completion and original channel/stream lifecycle tests. Whole retail PCM stream completion, explicit stop, original three-stream/two-ARAM-buffer capacity/reuse and cancellation passed component tests. The latest manager fixes correct a false failure return and permit idle DVD-file-info reuse while protecting pending reads. These checks use deterministic worker fences; they do not prove real-time scheduling, the complete JKR audio-thread loop, device playback or in-game audio.

The latest manager/DVD full DLL build was stopped when the user paused development. Its full integration remains unverified. Earlier DLL-only links were verified as development artifacts, but no matching newly verified launcher/DLL package is available for publication. A separate newer launcher has an unresolved Defender detection and is not run or distributed; see [artifact review](../native/ARTIFACT-REVIEW.json).

The published ZIP remains the hash-verified **October 27** launcher/DLL pair and unchanged standalone probe. Its recorded failure is the unregistered private display-list buffer during Mario drawing. It is historical diagnostics, not the current source built into a playable game. The November 1 publisher verified the ZIP and all eight member hashes without running its launcher. Probe hash is unchanged; its last publication self-test PASS remains October 18. See [build manifest](../builds/manifest.json).

Read the [verification scope](verification/2026-11-01-runtime-audio.md) and [daily check](DAILY-STATUS.md). Raw logs, dumps, game images/assets, HD packs and saves are excluded.

Rough whole-project estimate remains **about 3%**, not measured. Verified playable progress remains **0%**. Settings, HD integration and multiplayer remain future work.
