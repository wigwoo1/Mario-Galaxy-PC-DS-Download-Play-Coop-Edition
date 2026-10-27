# Daily publication check — 2026-10-27

Additional check at 14:43 UTC, following the newer local development work. The delayed 2026-10-26 trigger was processed against today's actual files; no older date is being represented as a new test.

- Reviewed 22 changed/new native source, test and progress files against the remote source manifest. Existing local Petari edits and provenance remain included; unchanged source is preserved. No credentials were found in the reviewed updates. Logs, build caches, game assets and saves are excluded.
- Published native BRK resource conversion, host numeric conversion fallbacks, per-draw TEV color constants/shader reuse, diagnostic-mode guards and a later BDL mutable-color-storage repair with its focused test. The material fixture passes two real animation attachments, 18 register checks and nine Mario material GPU checks. The broader original-model regression and a full runtime rebuild with that last repair remain pending. No development was resumed by this publication job.
- Packaged the existing 2026-10-27 diagnostic launcher/DLL pair, verified against its original runtime manifest and again inside the ZIP. This binary predates the final material-storage repair in current source. It is not a fresh rebuild by the publisher and is not playable.
- The standalone probe is unchanged; its self-test was not repeated. Last publication self-test PASS: 2026-10-18.
- Latest original runtime selected saved slot 1, accepted Start, loaded profile data and passed the former Kinopio BRK-start failure, then crashed during Butterfly initialization with C0000005. No completed stage frame or playable gameplay is verified. The actual save verifier passed and both before/after save hashes matched.
- Publication leaves the native-development goal status unchanged. Overall project estimate remains approximately 3% (rough planning estimate); verified playable completion remains 0%.
