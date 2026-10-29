# Daily publication check — 2026-10-29

Checked at 2026-10-29T13:02:12.029Z.

- Code and diagnostic builds have not changed since the October 28 publication check (b6e94ac8d427300781d212c96c487b915776e435). Compared SHA-256 hashes for 6,876 locally present manifest-listed source files, including locally modified Petari source, and checked the selected native/Petari source inventory for additions. No changed or new source was found. Existing provenance/notices remain unchanged.
- Existing launcher, DLL, runtime manifest and standalone probe hashes match the published build manifest. No binaries were rebuilt or repackaged. The probe is unchanged, so its self-test was not repeated; last publication self-test PASS: 2026-10-18.
- Current package remains development diagnostics, not playable. Last recorded runtime reached SCENE READY and then failed on an unregistered private display-list buffer during normal Mario drawing. No complete stage frame or controllable gameplay is verified. See [recorded verification](verification/2026-10-27-scene-factory-extra-matrices.md).
- This publication job only updates the daily check and source-manifest metadata. No development or runtime tests were resumed; the native-development goal remains paused as found. No game images/assets, HD packs, saves, credentials, logs, dumps, compilers or caches are published.
- Rough whole-project estimate remains about 3% (planning estimate, not measured); verified playable completion remains 0%.
