# Daily publication check — 2026-10-28

Checked at 2026-10-28T12:57:57.703Z.

- Code and diagnostic builds have not changed since commit 137841e88c82045501656f7da6a95b0531aa57de. Compared SHA-256 hashes for 6,876 locally present manifest-listed source files and checked the native source inventory for additions. Existing locally modified Petari source and provenance remain published unchanged.
- Existing launcher, DLL, runtime manifest and standalone probe hashes match the published build manifest. No binaries were rebuilt or repackaged. The probe is unchanged, so its self-test was not repeated; last publication self-test PASS: 2026-10-18.
- Current package remains development diagnostics, not playable. Last recorded runtime reached SCENE READY and then failed on an unregistered private display-list buffer during normal Mario drawing. No complete stage frame or controllable gameplay is verified. See [recorded verification](verification/2026-10-27-scene-factory-extra-matrices.md).
- This publication job only updates the daily check and source-manifest metadata. No development or runtime tests were resumed; the native-development goal remains paused as found. No game images/assets, HD packs, saves, credentials, logs, dumps, compilers or caches are published.
- Rough whole-project estimate remains about 3% (planning estimate, not measured); verified playable completion remains 0%.
