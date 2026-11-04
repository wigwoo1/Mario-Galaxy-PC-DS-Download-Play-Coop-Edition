# Current status — November 4, 2026 publication

**Not playable. Development is paused by the user.**

Current source is ahead of the historical October 27 diagnostic ZIP. A development run captured a partial Star Festival stage view before a camera-animation crash; no complete stage frame, reliable player control, campaign playthrough or full-fidelity rendering is verified. Camera decoding/ownership has component tests, but its full-game repair retest remains pending.

The previously interrupted manager/DVD DLL build completed on November 4. Original manager reuse/cancellation and native idle DVD-file-info reuse passed checks. A reproduced DAC bug truncated 64-bit PC buffer pointers into unmapped 32-bit DMA addresses; native generation now maps complete spans, and the full build refreshes that provider. A subsequent DLL-only checkpoint and standalone complete original JKR/JAS audio-thread tests passed within their scopes.

Retail streaming then ran through the actual audio and DVD workers without manual DSP stepping or playback worker fences. Natural completion, reuse/stop(10), cancellation and pool reclamation passed. **Playback speed did not:** natural completion took 46.744 seconds for approximately 26.4 seconds of source audio. A queue/owned-PCM edit in current `native/src/native_audio_ai.cpp` passed only the basic AI regression before work stopped. Correct tempo, queue ownership/restart/fallback, complete-thread regression and the full DLL build of this latest edit remain unverified. The retail wrapper's broad deadline still permits the slow result and must be tightened on development resume.

The latest verified DLL-only checkpoint predates the queue edit; it is not packaged as a runnable game. A separate newer launcher has an unresolved Defender detection and is excluded; see [artifact review](../native/ARTIFACT-REVIEW.json). No held binary or substitute launcher was run by publication.

**Packaged builds are unchanged:** the hash-verified October 27 launcher/DLL pair and standalone probe. Its recorded failure remains the Mario private display-list drawing-buffer fault. All eight ZIP member hashes and its embedded pair manifest were reverified. The probe is unchanged; its last publication self-test PASS remains October 18. See [build manifest](../builds/manifest.json).

Read the [November 4 evidence and limitations](verification/2026-11-04-audio-thread-and-timing.md), [older November 1 evidence](verification/2026-11-01-runtime-audio.md) and [daily check](DAILY-STATUS.md). Raw logs, dumps, images/assets, HD packs, saves, credentials, compilers and caches remain excluded.

Rough whole-project estimate remains **about 3%**, not measured. Verified playable progress remains **0%**. Settings, HD integration and multiplayer remain future work.
