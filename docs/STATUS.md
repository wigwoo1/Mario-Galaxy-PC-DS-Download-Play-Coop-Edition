# Current status — November 4, 2026 runtime-source publication

**Not playable. Development remains active.** Publication leaves goal status unchanged and does not resume development or execute tests.

Current source is ahead of the historical October 27 diagnostic ZIP. The last actual game run captured a partial Star Festival view before a camera-animation crash. Complete stage rendering, reliable player control and campaign completion remain unverified. Camera decoding/ownership has component tests; the full-game retest awaits legitimate review of a separately held launcher.

Completed development component evidence now covers pool/heap lifetimes, DVD/audio/stream restart across released heaps, actual sound-catalog stream requests, native track defaults/list offsets, original sequence-note startup, a real sampled instrument and three natural requests of the unmodified SE_SY_COIN sequence. The coin integration uses actual bank 64/waveform system 2 and restores original resource capacities after each cycle. This is not gameplay, complete spatial/audio policy, exact PCM fidelity or endpoint listening verification.

The shared JAUSoundMgr routing test is **incomplete**: its latest existing compile succeeds, but the requested concurrent stream remains preparing and the runtime completion assertion fails after three successful coin cycles. That source is published as work in progress, without a pass claim. Publication did not rerun or fix it.

Latest completed DLL-only checkpoint: 10,334,720 bytes, SHA-256 `F6AC7F11DB9659F7B689598C73003711B4B71AA0A18B88619B9343E70E05776E`. Existing incremental full-link receipts cover nineteen refreshed providers, seven sound-ID consumers, sixteen sampled/transfer sources and eight SE/request/sequence sources. Publication independently rehashed existing evidence and parsed 23 changed PowerShell scripts. It did not compile, execute development tests, load a DLL or launch the game. Clean-checkout full rebuilding remains unverified.

**Packaged builds have not changed.** All eight members of the October 27 ZIP and its embedded verified launcher/DLL pair were rehashed. That historical diagnostic retains the recorded Mario drawing-buffer fault. The standalone probe is unchanged; last publication self-test PASS remains October 18. New DLL-only checkpoints remain unpackaged without a matching verified launcher. The separately held launcher is excluded pending legitimate review; no restoration, rename, execution or substitute was used.

Read [latest verification](verification/2026-11-04-audio-lifetimes-and-retail-coin.md), [daily check](DAILY-STATUS.md) and [build manifest](../builds/manifest.json). Existing Petari edits/provenance/notices are preserved. Game images, extracted assets, HD packs, saves, credentials, raw logs, dumps, compilers and caches are excluded. Rough overall planning estimate **3%**, not measured; verified playable completion **0%**. Settings, HD integration and multiplayer remain future work.
