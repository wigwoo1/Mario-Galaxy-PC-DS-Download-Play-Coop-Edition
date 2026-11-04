# November 4: sound routing, music allocation and catalog initialization

Status: verified scoped development checkpoints; overall port partially verified and **not playable**. Publication inspected existing receipts and rehashed retained evidence. It did not compile, run development tests, load a DLL or launch the game.

## Completed development checks

- The original starter/router plays the real coin sound and prologue stream together. Three natural coin cycles, 694 overlapping subframes, 906 sampled-voice and 882 DSP-source checks passed. The stream finishes in 26.496 seconds against 26.308 seconds nominal within the 95%-110% gate. The earlier failure was fixture ordering: initialize stream block size before allocating two 362,880-byte ARAM buffers. No engine workaround was needed.
- Actual BGM_MISS (01000002) loads resource 80, smg_jingle01.szs, from the original JaiSeq archive: 640 compressed bytes and 864 decoded bytes. Three natural playback/reload cycles pass through original sequence/DVD/audio workers, real banks 0 and 1, DSP and native output. Both complete wave transfers match independent DVD reads. This verifies one short theme, not the music catalog.
- Original dynamic allocation now creates three 56,000-byte section-owned blocks. Native metadata and ownership links use their original JKR heap instead of the CRT allocator. Original static allocation deep-copies 864 sequence bytes. Playback/borrowed-region checks and complete coordinated worker/heap/backing cleanup pass. Original capacities remain unchanged. Allocation failures and concurrent teardown remain unverified.
- Original section factory and filename catalog loader expand the actual 596,320-byte SMR archive and create heap-owned BST/BSTN/BSC copies. All 48 bank/wave-system mappings and 50 waveform archive objects are checked. The authored startup mask loads one 119,712-byte waveform archive, independently byte-compared. Temporary archive storage is reclaimed at the same address and overwritten; catalog names, sequence offsets, copied bytes and an instrument lookup survive.
- The full runtime now selects the native expanded/bounded catalog filename loader. The latest completed incremental DLL-only checkpoint is 10,335,744 bytes, SHA-256 A41101EF706A0FB9F48025A038D6CA94E0573132CFF47C2599E2E974BDFF932E. Its receipt verifies 20 refreshed audio provider pairs, 13 sound/resource pairs and 16 sampled/8 SE/2 music direct-source receipts. It is an incremental link using other existing caches, not a clean-checkout full-game rebuild.

Publication rehashed 1,868 retained evidence files across the four completed checkpoints, including their DLL-only outputs. All eight changed PowerShell scripts parse. Those checks inspect existing results; they are not fresh development test runs.

## Current work in progress

The newly published AudSystem-construction fixture compiles and constructs the actual system with chord, ME and remix archives. Its observed construction checks include section ownership, two stream buffers, 40 ME entries and six remix groups. Its run still crashes during sound-object cleanup. The latest recorded cleanup-order edit has not produced a passing end-to-end result. Full AudSystem initialization, playback, shutdown and gameplay are **not verified**.

The AudSystem allocation/generator edits are newer than the completed DLL checkpoint above. Do not treat that DLL as validation of all current source. No new verified launcher/DLL pair is available. Source is published so the unfinished state can be reviewed, with no successful shutdown claim.

## Download and remaining limits

The October 27 diagnostic ZIP is unchanged. Publication rechecked its archive SHA-256, all eight member hashes and its embedded launcher/DLL manifest. Its standalone probe binary is unchanged; the last publication self-test PASS remains October 18. The ZIP retains its historical Mario drawing-buffer failure.

Current game development last captured a partial Star Festival view before a camera-animation fault. Full-game retesting is still gated by legitimate security review of a separate held launcher. That launcher is excluded; no replacement, restoration, rename, execution or protection change occurred.

Exact audio/visual fidelity, endpoint listening, full scene transitions, reliable controls and campaign play are unverified. Requested settings, HD integration and multiplayer remain future work. Development goal remains active and unchanged. Whole-project planning estimate remains roughly 3%, not measured; verified playable completion is 0%.
