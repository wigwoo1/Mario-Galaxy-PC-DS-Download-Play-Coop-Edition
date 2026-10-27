# Mario Galaxy PC — native port development

Experimental Windows x64 native port based on [SMGCommunity/Petari](https://github.com/SMGCommunity/Petari). **This is not a playable game.** The repository name describes planned co-op and browser Download Play, which are not implemented.

## Current state

The native diagnostic selects an existing Mario save slot, accepts Start, initializes Mario and passes the former bloom scene-object failure. It now passes that Kinopio animation start, then crashes during Butterfly initialization. Current source includes a later material-storage repair with a passing focused fixture; full-game rebuild and retest remain pending. No completed stage rendering, player-controlled gameplay, campaign playthrough or multiplayer is verified. Rough overall progress is about 3%, an engineering estimate.

See [current status](docs/STATUS.md), [verification](docs/verification/2026-10-27-native-color-animation.md) and [native progress](native/PROGRESS.md). Older native documents describe historical milestones.

## Download current diagnostic builds

[Download Windows x64 diagnostics](https://github.com/wigwoo1/Mario-Galaxy-PC-DS-Download-Play-Coop-Edition/raw/refs/heads/main/builds/windows-x64-diagnostics.zip). Extract the ZIP, then double-click **Run diagnostic self-tests.cmd**. It reports PASS or FAIL and keeps the window open. This test does not require a game image and does not launch gameplay.

The ZIP also contains the exact 2026-10-27 boot diagnostic launcher/DLL pair and its original manifest. It is a hidden research diagnostic with the known stage-loading crash, not a normal game launcher. The separate galaxy-probe executable is an existing diagnostic artifact. The runtime predates the last material-storage repair and is not a fresh rebuild by the publisher. [Build hashes](builds/manifest.json).

No game image, extracted asset directory, texture pack, user save, crash dump or compiler cache is included. Disc-backed tools require your own locally supplied game data.

## Source and building

- native/: current native code, adapters, generators, tests and roadmaps.
- Petari/: local upstream source snapshot including native-port edits; revision 0d9b80c569b954562c80206113e6936378c90c27.
- [BUILDING.md](BUILDING.md): dependencies and current reproducibility limits.
- [SOURCE-MANIFEST.json](SOURCE-MANIFEST.json): publication file hashes.
- [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md): licenses and provenance.

## Roadmap

Working single-player comes first. Later plans include mouse Wii pointing and keyboard/controller shake; resolution up to 8K and 120/240 FPS with correct game speed; stereo/5.1; HD textures/HUD; and Luigi co-op with independent cameras, P1 cutscene/NPC regrouping, Host/Join, P2P/LAN and browser Download Play with transfer progress. These are future features.

Daily publication checks run from the development computer and publish available changes plus truthful status. They do not invent progress or restart paused development. The computer and Codex must be available for local scheduled publication.

Independent project; not affiliated with Nintendo.
