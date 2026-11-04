# November 4 audio/runtime verification scope

> Historical earlier-day checkpoint. Its pending queue/timing and paused status are superseded by the [later November 4 verification](2026-11-04-audio-timing-pause-pool.md). Retained below as the original record.

This note summarizes inspected local receipts and logs. Raw logs, binaries outside the existing diagnostic ZIP and retail assets are excluded. The publication job did not rerun development tests or resume paused development.

## Completed checkpoints

- The interrupted original JAI manager/DVD milestone completed its DLL-only full link: 10,327,552 bytes, SHA-256 `E17274241529EF846D34F58564E0E530E6AFFA89B752E1C3027D543CD4335BD2`. Fifteen refreshed providers and the corrected manager/native DVD selection were checked. Original three-stream/two-ARAM-buffer ownership, natural EOF, reuse and cancellation checks passed.
- A reproduced original DAC call-site defect passed 64-bit PC pointers to a 32-bit DMA API. Native generation now registers full spans; exact initialization, 30 triple-buffer rotations/stereo sample checks, overlapping-map rejection and heap cleanup passed. The boot wrapper now replaces its cached `JASAiCtrl.obj` with the current complete native provider. The resulting DLL-only full build is 10,328,064 bytes, SHA-256 `6677955CFE59E4133370585D3389DBCBDA9F37D934BCA6F758358B20F6327F0A`. Sixteen refreshed source/object pairs and exact-once selection, extra DSP/manager providers, raw DVD selection and inert x64 PE/export structure were verified. **This artifact predates the queue edit below.**
- The complete original JKR/JAS audio thread ran on the actual native worker with native queues, DAC/AI and renderer. Three integration groups passed startup, 30 DAC updates/205 DSP subframes with synthetic PCM and natural voice EOF, and original STOP/AI join with no late DAC calls over an 80 ms observation. Every nonzero stereo chunk matched a scalar FIR/bus/output reference including edge transients. Seventy-three inputs were hash-checked/snapshotted. This uses a synthetic low-level voice and does not verify retail sound-info/spatial strategy or audible endpoint fidelity.

## Retail lifecycle succeeded; playback tempo failed

The actual `SMG_ev_prolo04_strm.ast` stream from the locally supplied WBFS contains 841,863 samples/channel. Real original audio/DVD workers and JAI manager completed natural EOF, reuse/stop(10), early cancellation and pool reclamation without manual DSP stepping or playback worker fences. Recorded natural run: 1,510 manager frames; last observed source position 841,816; 1,681,163 nonzero DAC samples; 790,718 stereo differences. Original three stream slots, two ARAM buffers, 72 parent slots and 64 DSP slots were preserved. Stop(10) drained after 13 manager frames.

**Natural playback took 46.744 seconds for approximately 26.4 seconds of nominal audio. Correct tempo is not verified.** The test returned success for lifecycle and its broad 55-second deadline; that must not be described as a timing pass. Its timing expectation needs tightening before retesting.

## Current source pending verification

`native/src/native_audio_ai.cpp` SHA-256 `4515A4FBACD0EEDFD3E67752441BCE052B61B5F529AC67E8EA16D045694CFE25` contains a queued Windows output/owned-PCM edit intended to remove inter-buffer gaps. It was applied after the user's pause signal; work then stopped. **Only the basic native AI regression passed afterward.** No retail timing retest, complete-thread regression, rigorous queue ownership/stop/restart/fallback validation or full DLL rebuild of this edit is recorded. Do not call it a verified fix.

Development remains paused. On explicit resume, review this edit, add a DMA cadence check that rejects the prior roughly 31 ms per 17.5 ms chunk behavior, tighten retail timing expectations, rerun thread/retail checks and verify a full build before making broader claims. Full JAI tables, spatial strategy, pause/resume, restart/overflow recovery, generic-pool static teardown, audio fidelity, the pending camera/game retest and playability remain unverified.

## Published artifacts

The runnable diagnostic ZIP remains the unchanged October 27 launcher/DLL pair with its recorded Mario drawing-buffer fault. Newer DLL-only checkpoints are not paired with a newly verified launcher and are not packaged. The launcher under unresolved Defender review remains excluded. The publisher reverified archive/member/pair hashes without executing a launcher or DLL. No game images/assets, texture packs, saves, credentials, raw logs or dumps are included.
