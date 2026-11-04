# November 4 follow-up: timing, pause and pool destructor

This note summarizes existing local development receipts inspected and hash-checked by publication. Publication did not resume development, compile, run tests, load a DLL or launch a game. It leaves the already-active native-development goal unchanged. This supersedes the earlier November 4 note's pending queue/timing status; earlier measurements remain historical evidence.

## Queued output and retail timing

The Windows output path keeps three owned PCM chunks queued; completion is based on actual device headers. Buffer submission copies caller PCM before returning. Separate tests passed with the caller's span unmapped and PAGE_NOACCESS before worker startup. Actual 32 kHz cadence was 0.689903 seconds for 39 intervals (0.6825 expected), with 22,400 device frames completed; 48 kHz was 0.663486 seconds (0.65 expected), with 32,000 device frames. Both reached three queued chunks. Stop/join, no late callbacks, callback-initiated stop with deferred control-thread reaping, and later restart passed. A separate test-only no-device backend exercised sample-clock fallback; it is absent from the production link. The physical owned-buffer check was skipped only in the deliberate no-device run and passed on the actual endpoint.

The complete original audio/DVD workers played the user's 841,863-sample/channel, 32 kHz retail stream through JAI/parent/DSP/native DAC/output with a one-entry BSFT fixture. No manual DSP stepping or playback fences were used. Natural completion took **26.410 seconds** for **26.30821875 seconds nominal**, replacing the earlier **46.744-second** result. The new 95%–110% timing gate rejects the old result. Counters: 1,510 manager frames, maximum source position 841,816, 1,681,163 nonzero DAC samples and 790,718 stereo differences. Reuse, stop(10) draining in 13 frames, early cancellation and restoration of three stream slots/two ARAM buffers/72 parent slots/64 DSP slots passed. Synthetic whole-thread and DAC mapping regressions passed.

The subsequent DLL-only link passed: 10,330,112 bytes, SHA-256 `EDCA9DD6F01BDEB66D01266980295C0C627693B796C814CE64C32232430385DB`. Sixteen refreshed providers and exact-once selection, current raw AI/DVD providers, absent no-device stub and inert x64 PE/export structure were verified. This is a development checkpoint, not a new runnable package.

## Live sound/manager pause

Three cycles on the actual retail worker path passed individual sound pause, manager pause remaining effective after individual pause is cleared, resume of the same stream and stop(10) while manager-paused. Source held at 8,950 and resumed to 13,266; paused stop drained in 12 manager frames. One stream/one ARAM buffer/two parent and DSP voices remained owned during pause, with all original capacities restored after every cycle.

180 held DAC chunks / **201,600 stereo sample values** matched an independent reference derived from each voice's saved constant sample, the actual front-only automatic mixer sine table/gains and original DAC plane order. Paused output can be a nonzero constant; silence was not assumed. Actual workers/transports stopped and the thread list emptied. This tests sound/manager APIs, not the game pause menu or separate whole-thread suspension flag. Larger fixtures intentionally retain process-lifetime pool backing after worker cleanup.

## Matching-allocator pool destruction

The original Wii global delete[] dispatches to JKRHeap::free; native CRT delete[] does not own JASDram chunks. The exact generic-pool destructor with real JKR root/solid heaps reproduced Windows heap-corruption exit `C0000374` without a replacement global deallocator. Native heap generation now translates this call locally to `JKRHeap::free(pChunk, nullptr)`, preserving raw Petari source, pool layout, capacities and allocation behavior.

Four real destructor cycles passed with two allocation batches/five slots, exhaustion, nontrivial free-list order and 100 reuse operations each. The complete containing solid heap restored parent free size/integrity, and normal main returned zero. The separate three-slot allocation/exhaustion/reuse regression and live retail pause regression passed again. The original solid heap's individual-free warnings reflect its bump-allocation behavior.

Latest DLL-only link passed after this repair: **10,330,112 bytes**, SHA-256 **`E844A0BF91E32CB7098304FB5F09E77C7B7DE5FFBCF07A1F9874865D5FA2347C`**. Sixteen refreshed provider pairs, current raw heap/AI/DVD selection and extra DSP/manager providers were hash-checked and linked exactly once; stale heap objects/test backend were absent. Inert x64 PE/export checks passed. Four changed PowerShell scripts parsed, heap generation was deterministic, 28 component input hashes and 52 source snapshots were recorded. Publication independently rehashed the retained DLLs and the selected checkpoint evidence, without executing them.

This repair requires the allocating heap to remain alive and all slots to have been returned. Pool destruction after heap disposal, outstanding objects, complete system shutdown/restart, forced mid-play device loss/timeouts, concurrent control calls, full spatial sound-info, endpoint orientation/listening/fidelity, pending camera/game retest and gameplay remain unverified. No launcher was built for these DLL-only milestones.

## Publication scope

The diagnostic ZIP remains the unchanged October 27 verified launcher/DLL pair and probe, with its recorded Mario drawing-buffer fault. New DLL-only checkpoints have no matching verified launcher package. The separately held launcher remains excluded pending legitimate security review; no restoration, renaming, launcher substitution or protection change occurred. Archive/member/pair hashes were reverified. Raw logs, dumps, extracted game data, images, textures/HD packs, saves, credentials, compiler downloads and caches are excluded. Existing source/provenance/license notices are preserved.

Overall project planning estimate remains roughly **3%**, not measured; verified playable completion **0%**. Sol High remains appropriate for the next difficult runtime lifetime/integration task; this publication itself is routine maintenance.
