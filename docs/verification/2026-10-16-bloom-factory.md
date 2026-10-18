# Bloom factory and saved-slot verification

**Partial verification of the port.** Original factory cases now create ImageEffectSystemHolder, BloomEffect, BloomEffectSimple, ScreenBlurEffect and DepthOfFieldBlur. The original ScreenBlurEffect provider is added to the complete link; original scheduling, textures and draw code remain intact.

Full build passed: DLL 10,240,512 bytes; launcher 914,944 bytes. The manifest-verified runtime passed saved-slot hover/selection, FileConfirm and Start. After the post-Start scene initialization, Mario initialization and placement groups _104 and _100 completed, passing the former BloomArea exception.

The process then crashed with C0000005 after 1007.0 seconds in _108 NPC placement. Stack: AnmPlayerBase::start -> ModelManager::startBrk -> Kinopio::init -> PlacementInfoOrdered::initPlacement -> StageDataHolder::initPlacement -> GameScene::init. Kinopio requests ColorChange. The failed read at 0xA is consistent with a null animation read at mAnmRes->mFrameMax.

Scene-gated capture never armed. Progression beyond the factory failure does not verify bloom pixels or completed gameplay. Local saves passed verification and were unchanged. Raw logs, dumps and saves are excluded. The download contains the exact launcher/DLL and original manifest; builds/manifest.json records hashes.
