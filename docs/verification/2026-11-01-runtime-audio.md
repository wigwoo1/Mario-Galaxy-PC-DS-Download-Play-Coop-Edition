# November 1 development evidence and limits

This publication summarizes existing local development evidence. It did not resume development, compile code, run game diagnostics or change the paused goal.

## Rendering and camera

- Player display-list and flag-array repairs received focused tests. A later native development run reached SCENE READY and captured Mario and part of the Star Festival scene before a camera-animation access violation.
- A captured stage view is not a completed, controllable frame or verified campaign gameplay. The terminal fault moved to original CameraAnim position evaluation.
- Camera resource ownership and big-endian decoding received component tests against three retail camera records, including full- and half-frame samples. A subsequent game run stopped before exercising that camera. Full-game validation is still missing.
- A newer launcher was quarantined by Defender. Its classification has not been adjudicated; no restoration, renaming, old-launcher substitution or protection change was used. [Recorded hold](../../native/ARTIFACT-REVIEW.json).

## Audio components

- A Dolphin-derived Zelda audio renderer and frame executor now process real native JAS tables/voices, ARAM sample reads and effects rings. The original imports, hashes, adaptation generator and GPL notices are included in source.
- Focused tests cover synthetic PCM, AFC decoding, original coefficient tables, a retail PCM block, original command groups, bounded pointers, deferred SDK mail, natural EOF and effects routing. An independent absolute-DRAM oracle uses the actual SMG DSP image's effects addresses/history layout. This is not a complete DSP LLE or hardware-equivalence proof.
- Original DSP and parent JAS channels now pass natural completion, drop/replacement, capacity, allocation return and packed-mixer layout tests. The native little-endian mixer header preserves original packed bus/modifier values; automatic mixing no longer indexes a twelve-entry table with its 0xFF sentinel.
- Original JASAramStream and a real JKR DVD task worker played the complete non-looping stereo PCM stream SMG_ev_prolo04_strm.ast: 841,863 source samples per channel, 10,536 audio subframes to natural completion. Explicit stop completed after 39 subframes in its fixture. Original callbacks/destruction returned all 72 parent allocations and 64 DSP slots.
- The original JAIStream/JAIStreamMgr, JAU static ARAM manager and BSFT adapter passed four manager test groups: original three-object/two-buffer capacity and fourth-request rejection, waiting-stream handoff and ten playback/reuse/stop cycles, whole retail natural completion, preparation cancellation, and worker/runtime cleanup. Invalid-ID and exhausted requests remain failures; successful starts now return success. No stream slots were added and no finished/dead flags were fabricated.
- Manager natural completion took 1,510 fixture manager frames with seven audio subframes per frame; it released the handle, stream object and ARAM buffer. The fixture supplies a valid one-entry BSFT pointing to the retail stream, not the complete game sound-info/audience/strategy system.
- Reuse testing exposed a native DVD mismatch: original SDK DVDFastOpen permits reinitializing idle metadata. The native adapter now permits that reuse and still rejects pending read/callback ownership. Dedicated exact-byte/busy-callback tests, basic DVD and cancellation regressions passed. Prior stream regression groups also passed.
- Tests serialize loads with worker fences. Real-time scheduling, unrestricted thread races, full JKR audio-thread/DAC/device output, looped/AFC stream reuse and whole-game audio remain unverified. Individual frees from the solid heap intentionally report warnings; static-pool/whole-heap shutdown is not established by these tests.

## Build scope

Existing local DLL-only verification receipts include:

| Checkpoint | SHA-256 | Scope |
| --- | --- | --- |
| Parent mixer/lifecycle DLL | 5d6fdccafa2582789e7a7e22b90495ad3cd77eb7d5b14c23298394644fd4495c | Full DLL link and selected-provider hashes; no launcher or DLL execution |
| Stream automatic-route guard DLL | ae018beea590ad1ebc5052523ab7b9c71069aa884efb69d427d503db288671b6 | Full DLL link, fifteen tracked providers plus guarded DSP interface checked; no launcher or DLL execution |
| Latest manager/DVD changes | Not verified | User pause interrupted the full DLL build; component tests passed |

These DLL-only artifacts are not published as runnable packages. The existing October 27 ZIP is unchanged and predates these repairs. Its SHA-256 is ab7d25d98c2e79a73de660435e162e81620f8f78859a7ad1441c19e5c310b28c. The publisher rechecked all eight ZIP member hashes, including its original launcher/DLL pair, and confirmed the held launcher hash is absent. No new binary was rebuilt or repackaged for this publication.

Native progress/roadmap files retain historical notes. The last manager checkpoint was saved in local build/jai-stream-manager-lifecycle/CONTINUATION.json as paused, with full DLL verification unfinished; build directories and raw logs are not included in public source.
