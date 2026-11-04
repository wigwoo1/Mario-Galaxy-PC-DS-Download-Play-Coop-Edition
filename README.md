# Mario Galaxy PC — native port development

Experimental Windows x64 native port based on [SMGCommunity/Petari](https://github.com/SMGCommunity/Petari). **This is not a playable game.** The repository name describes planned co-op and browser Download Play, which are not implemented.

## Current state

The November 4 source includes verified stream-manager/DVD fixes, 64-bit DAC buffer mapping, a complete original audio-thread test and retail-stream integration tests. A development game run previously captured a partial Star Festival view before a camera-animation crash. Complete stage rendering, player-controlled gameplay, audible game output and campaign completion remain unverified.

**Development is paused. Current audio-output queue source is pending verification:** a retail stream completed and released its resources, but took 46.744 seconds for approximately 26.4 seconds of audio. A subsequent queue edit passed only the basic native AI test; playback timing, whole-thread regression and a full DLL build of that edit remain unverified. Do not treat the retail test's lifecycle PASS as correct playback speed.

Newer verified DLL-only checkpoints exist, but no matching new launcher/DLL package is published. The held launcher remains excluded pending security review. Rough overall progress remains about 3%, a planning estimate rather than a measured counter.

See [current status](docs/STATUS.md), [November 4 verification and pending work](docs/verification/2026-11-04-audio-thread-and-timing.md), [daily publication check](docs/DAILY-STATUS.md) and [native progress](native/PROGRESS.md). Progress/roadmap entries describe dated checkpoints; the pending queue edit is newer than their last passing test entry.

## Download existing diagnostic builds

[Download Windows x64 diagnostics](https://github.com/wigwoo1/Mario-Galaxy-PC-DS-Download-Play-Coop-Edition/raw/refs/heads/main/builds/windows-x64-diagnostics.zip). Extract the ZIP, then double-click **Run diagnostic self-tests.cmd**. It reports PASS or FAIL and keeps the window open. This test does not require a game image and does not launch gameplay.

The unchanged ZIP contains the exact October 27 boot diagnostic launcher/DLL pair and its original manifest, plus the standalone probe. The packaged runtime is a historical hidden research diagnostic with the recorded Mario drawing-buffer failure. It predates the November 1–4 repairs and is not a playable game or a fresh rebuild by this publisher. Newer DLL-only development artifacts and the held launcher are not packaged. [Build hashes and limitations](builds/manifest.json).

No game image, extracted asset directory, texture pack, user save, crash dump or compiler cache is included. Disc-backed tools require your own locally supplied game data.

## Source and building

- native/: current native code, adapters, generators, tests and roadmaps.
- Petari/: local upstream source snapshot including native-port edits; revision 0d9b80c569b954562c80206113e6936378c90c27.
- [BUILDING.md](BUILDING.md): dependencies and current reproducibility limits.
- [SOURCE-MANIFEST.json](SOURCE-MANIFEST.json): publication file hashes.
- [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md): licenses and provenance, including the vendored GPL-2.0-or-later Dolphin audio renderer and its adaptations.

## Roadmap

Working single-player comes first. Later plans include mouse Wii pointing and keyboard/controller shake; resolution up to 8K and 120/240 FPS with correct game speed; stereo/5.1; HD textures/HUD; and Luigi co-op with independent cameras, P1 cutscene/NPC regrouping, Host/Join, P2P/LAN and browser Download Play with transfer progress. These are future features.

Daily publication checks publish available changes and truthful status. They do not restart paused development. The development computer and Codex must be available for local scheduled publication.

Independent project; not affiliated with Nintendo.
