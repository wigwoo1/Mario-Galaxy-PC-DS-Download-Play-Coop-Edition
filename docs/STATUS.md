# Current status — 2026-10-18 publication

**Not playable.** Latest stage-start verification: 2026-10-16.

The full DLL/launcher links. An actual game-data test selects saved Mario slot 1, accepts Start, initializes Mario and passes the former bloom failure. Loading then crashes in AnmPlayerBase::start -> ModelManager::startBrk -> Kinopio::init while starting ColorChange.

The original BRK resource boundary needs investigation. No speculative animation skip has been applied. No completed stage capture, Mario control, audible game audio or campaign playthrough is verified. Actual local save verification passed and hashes stayed unchanged; no saves are published.

The standalone diagnostic self-test passed again during publication preparation on 2026-10-18. Rough requested-project estimate: about **3%**, not a measured counter. Verified playable progress: **0%**. Settings, HD integration and multiplayer remain future work.
