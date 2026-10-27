# Native color-animation verification — 2026-10-27

This is a publication summary of existing local evidence; the publishing job did not rebuild the game or resume development.

- Native BRK conversion uses the original animation sampler and material attachment API. Existing checks report 12 archives, 21 resources, 4,956 samples, 177 material matches and ownership/bounds/disposal checks.
- The renderer binds immutable per-draw TEV color constants. Existing GPU checks report 1,024 color variants sharing one shader, 187 reference pixel comparisons, alternating-draw isolation and a structural-change check. Both literal and uniform 16-stage paths pass 256 fragments; existing D3D model-draw regressions pass.
- The existing full diagnostic build passed: DLL 10,255,872 bytes; launcher 914,944 bytes. Its original saved-profile stage test ended after 953.9 seconds with C0000005 during Butterfly initialization, after passing Kinopio BRK start. No completed stage frame or gameplay was verified. Actual save verification passed with unchanged before/after hashes.
- A later focused test reproduced Butterfly's one-stage material missing mutable color storage. Current source retains full mutable TEV capacity while preserving the authored one-stage count. The repaired fixture passes Butterfly and Kinopio attachment/calc/removal, 18 register checks, authored stage counts and nine Mario material/GPU checks. This last change is **not in the packaged runtime**. Broader model regression and full-game retest remain pending.
- Diagnostic-mode preflight checks passed 11 cases. Read-only profile observation must not be combined with profile selection or Start input.

The archive contains only the unchanged standalone probe/self-test launcher, the exact existing runtime pair and manifest, a limitations README, and notices. All archive member hashes were checked locally before upload. No game image, extracted asset, HD pack, save, raw log, crash dump, debug symbols or compiler is included.

Playability is unverified. Overall project estimate remains roughly 3%.
