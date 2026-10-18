#include "native_runtime_client.hpp"
#include "layout_text_geometry.hpp"
#include "resource_font.hpp"
#include "native_matrix_tests.hpp"
#include "hash_sort_table_tests.hpp"
#include "native_string_tests.hpp"
#include "native_functor_tests.hpp"
#include "miss_ui_resource.hpp"
#include "message_text.hpp"
#include "layout_text_binding.hpp"
#include "layout_text_tests.hpp"
#include "layout_composition.hpp"
#include "layout_texture_sampling.hpp"
#include "layout_material_shading.hpp"
#include "layout_texture_transform.hpp"
#include "layout_window_mesh.hpp"
#include "layout_geometry.hpp"
#include "layout_pose.hpp"
#include "layout_resource.hpp"
#include "layout_animation.hpp"
#include "miss_layout_tests.hpp"

#include "Game/Util/ActorStateUtil.hpp"
#include "mario_walk_targets.hpp"
#include "pose_blend.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "actor_state_keeper.hpp"
#include "Game/System/NerveExecutor.hpp"
#include "wbfs.hpp"
#include "wbfs_tests.hpp"
#include "asset_tests.hpp"
#include "graphics_tests.hpp"
#include "preview.hpp"
#include "rig.hpp"
#include "rig_tests.hpp"
#include "mesh.hpp"
#include "skinning.hpp"
#include "mesh_render.hpp"
#include "material_tests.hpp"
#include "coin_rotation.hpp"
#include "coin_rewards.hpp"
#include "save_chunks.hpp"
#include "player_save_file.hpp"
#include "galaxy_progress.hpp"
#include "campaign_catalog.hpp"
#include "stage_memory.hpp"
#include "restart_cube.hpp"
#include "death_area.hpp"
#include "player_death_sequence.hpp"
#include "animation.hpp"
#include "animation_clock.hpp"
#include "animation_preview.hpp"
#include "stage_data.hpp"
#include "stage_placement.hpp"
#include "stage_scene.hpp"
#include "collision_geometry.hpp"
#include "collision_query.hpp"
#include "stage_collision.hpp"
#include "sphere_contact.hpp"
#include "collision_index.hpp"
#include "collision_response.hpp"
#include "collision_sweep.hpp"
#include "collision_slide.hpp"
#include "diagnostic_body.hpp"
#include "body_preview.hpp"
#include "point_gravity.hpp"
#include "disk_gravity.hpp"
#include "gravity_field.hpp"
#include "scenario_layers.hpp"
#include "scenario_assembly.hpp"
#include "scenario_scene.hpp"
#include "scenario_collision.hpp"
#include "scenario_gravity.hpp"
#include "name_obj_tests.hpp"
#include "jmap_native_tests.hpp"
#include "jmap_bridge.hpp"
#include "placed_actor.hpp"
#include "scenario_actors.hpp"
#include "sensor_runtime.hpp"
#include "sensor_world.hpp"
#include "sensor_info_tests.hpp"
#include "sensor_keeper.hpp"
#include "sensor_message_tests.hpp"
#include "live_actor_message_tests.hpp"
#include "stateful_sensor_host.hpp"
#include "scheduled_sensor_actor.hpp"
#include "actor_timing_tests.hpp"
#include "stateful_placed_actor.hpp"
#include "stateful_placement_render.hpp"
#include "stateful_placement_preview.hpp"
#include "bck_settings_tests.hpp"
#include "mario_animation_groups.hpp"
#include "mario_logical_preview.hpp"
#include "animation_quaternion.hpp"
#include "animation_track_blend.hpp"
#include <iostream>
#include <string>

static void require(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}
struct ProbeNerve : Nerve {
    mutable int calls = 0, ends = 0, lastStep = -99;
    const Nerve* next = nullptr;
    void execute(Spine* spine) const override {
        ++calls; lastStep = spine->mStep;
        if (next) spine->setNerve(next);
    }
    void executeOnEnd(Spine*) const override { ++ends; }
};
struct ProbeState : ActorStateBaseInterface {
    int appearances = 0, kills = 0;
    ProbeState() : ActorStateBaseInterface("native test") { mIsDead = true; }
    void appear() override { ActorStateBaseInterface::appear(); ++appearances; }
    void kill() override { ActorStateBaseInterface::kill(); ++kills; }
};
static void coreTests() { GameDataPlayerStatus status; status.mStoryProgress=3; status.addStockedStarPiece(123); auto file=encodeSaveChunks({&status}); require(file.size()==23&&file[0]==1&&file[1]==1&&be32(file.data()+4)==0x504c4159&&be32(file.data()+8)==0x27c90f&&be32(file.data()+12)==19,"Original save chunk header layout mismatch");
    GameDataPlayerStatus loaded; require(decodeSaveChunks(file,{&loaded})&&loaded.mStoryProgress==3&&loaded.mStockedStarPiece==123,"Player status save chunk round trip failed"); for(int failure=0;failure<3;++failure){auto bad=file;if(failure==0)bad.pop_back();if(failure==1)bad[8]^=1;if(failure==2)bad[15]=11;loaded.mStoryProgress=7;require(!decodeSaveChunks(bad,{&loaded})&&loaded.mStoryProgress==7,"Invalid chunk mutated player data");}  galaxyProgressTests(); campaignProgressTests(); galaxyProgressFieldTests(); campaignSaveTests(); stageResultTests(); stageMemoryTests(); restartCubeTests(); deathAreaTests(); playerDeathSequenceTests(); missLayoutTests(); missLayoutPlaybackTests(); layoutAnimationMetadataTests(); layoutTrackTests(); layoutPoseTests(); layoutGeometryTests(); layoutWindowMeshTests(); layoutTextureTransformTests(); layoutMaterialShadingTests(); layoutTextureSamplingTests(); layoutCompositionTests(); layoutTextResourceTests(); layoutTextBindingTests(); bmgTextTests(); resourceFontLookupTests(); nativeMatrixTests(); hashSortTableTests(); nativeStringTests(); nativeFunctorTests(); layoutTextGeometryTests(); playerStockTests(); coinRewardTests(); coinRotationTests(); coinPoseTests(); NativeScenarioActors::coinVisibilityTests(); NativeScenarioActors::coinCollectionTests();
    ProbeNerve first, second;
    NerveExecutor actor("native test");
    actor.initNerve(&first);
    actor.updateNerve();
    require(first.calls == 1 && first.lastStep == 0 && actor.getNerveStep() == 1, "First state tick failed");
    actor.setNerve(&second);
    require(first.ends == 1 && actor.isNerve(&second) && actor.getNerveStep() == -1, "Pending transition failed");
    actor.updateNerve();
    require(second.calls == 1 && second.lastStep == 0 && actor.getNerveStep() == 1, "Transition step reset failed");
    second.next = &first;
    actor.updateNerve();
    require(second.ends == 1 && actor.isNerve(&first) && actor.getNerveStep() == 0, "In-tick transition failed");
    actor.updateNerve();
    require(first.calls == 2 && first.lastStep == 0, "Following state failed");
    ProbeState state;
    NativeActorStateKeeper keeper(1);
    keeper.addState(&state, &first, "probe");
    keeper.startState(&second);
    require(!keeper.updateCurrentState(), "Missing state should be inactive");
    keeper.startState(&first);
    require(state.appearances == 1 && !keeper.updateCurrentState(), "State appear/update failed");
    keeper.endState(&first);
    keeper.endState(&first);
    require(state.kills == 1 && keeper.updateCurrentState(), "State termination failed");
    for(int capacity:{0,-1}){bool rejected=false;try{NativeActorStateKeeper invalid(capacity);}catch(const std::exception&){rejected=true;}require(rejected,"Invalid actor state capacity accepted");}
    for(int failure=0;failure<3;++failure){bool rejected=false;try{keeper.addState(failure==0?nullptr:&state,failure==1?&first:&second,"probe");}catch(const std::exception&){rejected=true;}require(rejected,"Invalid actor state registration accepted");}
    keeper.startState(&first);require(state.appearances==2&&!keeper.updateCurrentState(),"Rejected registration corrupted original state lifecycle");
    ProbeNerve stateFirst,stateSecond;ProbeState firstState,secondState;Spine transition(nullptr,&stateFirst);
    {NativeActorStateKeeper transitions(2);transitions.addState(&firstState,&stateFirst,"first");transitions.addState(&secondState,&stateSecond,"second");transitions.attachTo(transition);transitions.startState(&stateFirst);transition.setNerve(&stateSecond);transition.update();require(firstState.kills==1&&secondState.appearances==1,"Original Spine state transition did not dispatch lifecycle");transition.setNerve(&stateFirst);transition.update();require(secondState.kills==1&&firstState.appearances==2,"Original Spine return transition failed");}
    require(transition.mStateKeeper==nullptr,"Destroyed state owner left Spine dangling");transition.setNerve(&stateSecond);transition.update();
    struct CompletingState:ProbeState{bool complete=false;int updates=0;bool update()override{++updates;return complete;}} completing;
    LiveActor stateHost;ProbeNerve updateNerve,nextNerve;Spine stateSpine(&stateHost,&updateNerve);stateHost.mSpine=&stateSpine;
    require(!MR::updateActorStateAndNextNerve(&stateHost,&completing,&nextNerve)&&completing.appearances==1&&completing.updates==1&&completing.kills==0,"Original state first-step update mismatch");
    stateSpine.update();completing.complete=true;
    require(MR::updateActorStateAndNextNerve(&stateHost,&completing,&nextNerve)&&completing.appearances==1&&completing.kills==1&&stateHost.isNerve(&nextNerve),"Original state completion transition mismatch");stateSpine.update();require(nextNerve.calls==1,"Completed state did not enter next nerve");stateHost.mSpine=nullptr;
    NerveExecutor managed("state helper probe");ProbeNerve managedFirst,managedNext;managed.initNerve(&managedFirst);CompletingState managedState;
    {NativeActorStateKeeper managedKeeper(1);managedKeeper.addState(&managedState,&managedFirst,"managed");managedKeeper.attachTo(*managed.mSpine);managedKeeper.startState(&managedFirst);require(!MR::updateActorStateAndNextNerve(&managed,&managedNext)&&managedState.updates==1,"Incomplete managed state transitioned");managedState.complete=true;require(MR::updateActorStateAndNextNerve(&managed,&managedNext)&&managed.isNerve(&managedNext),"Managed state did not request next nerve");managed.updateNerve();require(managedState.kills==1&&managedNext.calls==1,"Managed state transition cleanup failed");require(!MR::updateActorState(&managed),"Unregistered next state was active");}
    NerveExecutor explicitExecutor("explicit helper probe");ProbeNerve explicitFirst,explicitNext;explicitExecutor.initNerve(&explicitFirst);CompletingState explicitState;require(!MR::updateActorState(&explicitExecutor,&explicitState)&&explicitState.appearances==1,"Executor first-step state helper failed");explicitExecutor.updateNerve();explicitState.complete=true;require(MR::updateActorStateAndNextNerve(&explicitExecutor,&explicitState,&explicitNext)&&explicitState.kills==1,"Executor explicit completion helper failed");
    struct InitializingState:ProbeState{int inits=0;bool fail=true;std::function<void()> action;void init()override{++inits;if(action)action();if(fail)throw std::runtime_error("expected state init failure");}} initialized;
    NativeActorStateKeeper initialization(1);ProbeNerve initializationNerve;bool initRejected=false;try{initialization.initState(&initialized,&initializationNerve,"initialized");}catch(const std::exception&){initRejected=true;}require(initRejected&&initialized.inits==1,"State initialization failure not propagated");initialized.fail=false;initialized.action=[&](){bool blocked=false;try{initialization.addState(&initialized,&initializationNerve,"recursive");}catch(const std::exception&){blocked=true;}require(blocked,"State init registered recursively");};initialization.initState(&initialized,&initializationNerve,"initialized");initialization.startState(&initializationNerve);require(initialized.inits==2&&initialized.appearances==1,"Failed initialization consumed capacity");initRejected=false;try{initialization.initState(&initialized,&initializationNerve,"duplicate");}catch(const std::exception&){initRejected=true;}require(initRejected&&initialized.inits==2,"Rejected registration ran state init");
    std::cout << "PASS: native Petari Nerve, Spine, NerveExecutor, ActorStateKeeper and ActorStateBase\n";
}
static void discInfo(const std::filesystem::path& path) {
    Wbfs disc(path);
    auto h = disc.read(0, 0x100);
    require(be32(h.data() + 0x18) == 0x5d1c9ea3, "Wii disc magic not found");
    std::string id(reinterpret_cast<char*>(h.data()), 6);
    std::cout << "Disc ID: " << id << "\nTitle: ";
    for (size_t i = 0x20; i < h.size() && h[i]; ++i) std::cout << char(h[i]);
    std::cout << "\n";
    auto groups = disc.read(0x40000, 32);
    size_t games = 0;
    for (int g = 0; g < 4; ++g) {
        auto count = be32(groups.data() + g*8);
        uint64_t offset = uint64_t(be32(groups.data() + g*8 + 4)) * 4;
        require(count <= 1024, "Invalid partition count");
        auto table = disc.read(offset, count * 8);
        for (uint32_t i = 0; i < count; ++i) {
            uint64_t start = uint64_t(be32(table.data() + i*8)) * 4;
            uint32_t type = be32(table.data() + i*8 + 4);
            auto p = disc.read(start, 0x2c0);
            uint64_t dataOffset = uint64_t(be32(p.data() + 0x2b8)) * 4;
            uint64_t dataSize = uint64_t(be32(p.data() + 0x2bc)) * 4;
            require(start <= Wbfs::capacity && dataOffset <= Wbfs::capacity - start && dataSize <= Wbfs::capacity - start - dataOffset, "Invalid partition extent");
            games += type == 0;
            std::cout << "Partition type " << type << ": offset=" << start << ", encrypted data bytes=" << dataSize << "\n";
        }
    }
    require(games > 0, "No game partition found");
    std::cout << "PASS: WBFS logical mapping and partition metadata readable.\n"
              << "This does NOT verify/decrypt game assets or boot the game.\n";
}
static void saveNewResource(const std::filesystem::path& path, const std::vector<uint8_t>& bytes) {
    HANDLE file = CreateFileW(path.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file == INVALID_HANDLE_VALUE) throw std::runtime_error("Cannot create resource output (destination must not already exist)");
    DWORD written=0;
    BOOL ok=WriteFile(file,bytes.data(),static_cast<DWORD>(bytes.size()),&written,nullptr);
    BOOL closed=CloseHandle(file);
    assetCheck(ok && closed && written==bytes.size(), "Resource write failed; partial output may exist");
}
static std::vector<uint8_t> loadLocal(const std::filesystem::path& path) {
    uint64_t size=std::filesystem::file_size(path);assetCheck(size<=256*1024*1024,"Local input too large");
    std::ifstream file(path,std::ios::binary);std::vector<uint8_t> bytes(static_cast<size_t>(size));
    assetCheck(bool(file.read(reinterpret_cast<char*>(bytes.data()),static_cast<std::streamsize>(size))),"Local input read failed");return bytes;
}
int main(int argc, char** argv) {
    try {
        if (argc == 2 && std::string(argv[1]) == "--self-test") {
            coreTests(); wbfsTests(); assetTests(); graphicsTests(); rigTests(); meshTests(); skinningTests(); materialTests(); animationTests(); animationClockTests(); stageDataTests(); placementTests(); collisionQueryTests(); placedCollisionTests(); sphereContactTests(); collisionResponseTests(); collisionSweepTests(); collisionSlideTests(); diagnosticBodyTests(); pointGravityTests(); diskGravityTests(); gravitySelectionTests(); boxGravityTests(); scenarioLayerTests(); scenarioGravityTests(); nameObjTests(); nativeSceneTests(); nativeJmapTests();
            stageSwitchStateTests();stageSwitchWatcherTests();originalAppearanceListenerTests();originalSwitchFunctorTests();diagnosticHeadingTests();std::cout<<"PASS: diagnostic heading diagonal normalization, idle retention, gravity projection and degenerate fallback\n";
            collisionResourceTests();std::cout<<"PASS: named main collision selection, ambiguous archive rejection and single-resource compatibility\n";
            factoryArchiveTests();std::cout<<"PASS: original factory primary archive aliases and case lookup\n";
            mipmapTests();std::cout<<"PASS: BTI mip dimensions, offsets, pixels and invalid/truncated chain rejection\n";
            segmentGravityTests();std::cout<<"PASS: segment gravity side/endpoints, disabled edges, range, inverse and sectors\n";
            scenarioStartTests();std::cout<<"PASS: MarioNo start lookup, missing IDs and repeated-zone ambiguity\n";
            diagnosticMotionTests();std::cout<<"PASS: original NerveExecutor diagnostic motion first-step/transition timing\n";
            poseBlendTests();std::cout<<"PASS: diagnostic pose interpolation, shortest-angle wrap, endpoints and invalid weights\n";
            marioDecideInertiaTests();marioDefaultSpecialInertiaTests();marioSpecialInertiaTests();marioDefaultStopInertiaTests();marioStopInertiaTests();marioDefaultInertiaTests();marioInertiaTests();marioWalkSpeedTests();marioTargetSpeedTests();marioDefaultCadenceTests();marioWalkCadenceTests();marioStartRateTests();marioSlopeRateTests();marioWalkFinalRateTests();marioWalkRateTests();nativeStickTests();marioWalkIndexTests();marioWalkTargetTests();animationInterpolationTests();animationPoseHistoryTests();animationPoseStageTests();animationTrackBlendTests();std::cout<<"PASS: weighted animation track SRT/quaternion skinning, zero-weight fallback and invalid rig/weight rejection\n";animationQuaternionTests();std::cout<<"PASS: scalar JMA quaternion interpolation, Euler rotation agreement and antipodal/invalid cases\n";marioMultiGroupTests();marioAnimationNameTests();std::cout<<"PASS: original Mario animation names roundtrip UTF-8/Shift-JIS, encoded lookup and malformed-name rejection\n";bckSettingsTests();std::cout<<"PASS: original BCK clip settings, playback speed, loop, interpolation request and ignored/default fields\n";actorTimingTests();std::cout<<"PASS: original actor step helpers and timed scene-scheduled state transition\n";scheduledSensorActorTests();std::cout<<"PASS: scene-scheduled actor states, message reactions, suspend/resume and dead/clipped guards\n";liveActorNerveTests();std::cout<<"PASS: original actor nerve initialization, message-triggered transitions, end callbacks and step timing\n";liveActorMessageTests();std::cout<<"PASS: original LiveActor message dispatcher across 256 values, handler order, sender/receiver and reply propagation\n";sensorMessageTests();std::cout<<"PASS: original actor sensor contact messages, accepted/rejected replies and directional attack position restoration\n";sensorReclassificationTests();std::cout<<"PASS: runtime sensor reclassification, validity preservation, contact clearing and failed-capacity rollback\n";sensorRoutingTests();std::cout<<"PASS: original sensor type routing across 256 values and automatic player/item overlap\n";sensorPipelineTests();appearanceSensorLifecycleTests();std::cout<<"PASS: named sensor position-overlap-contact pipeline, stale contact clearing, validity, dead targets and shared capacity\n";sensorKeeperTests();nativeLifecycleNerveTests();std::cout<<"PASS: original sensor keeper named lookup, immediate/bulk updates, validity and contact dispatch with native bounded ownership\n";sensorInfoTests();std::cout<<"PASS: original sensor positions, matrix offsets, host callbacks and live contact dispatch\n";sensorRuntimeTests();std::cout<<"PASS: original HitSensor registration, validity, contacts, type changes and message dispatch\n";
            sensorWorldTests();std::cout<<"PASS: original sensor overlap, tangency, validity, clipping, same-host/eye rules and native bounded ownership\n";
            std::cout<<"PASS: original JMapInfo host tables, signed/float lookup, row search and JGadget hash\n";
            std::cout<<"PASS: original Petari NameObj registration, virtual initialization, movement, deferred suspend/resume and request cancellation\n";
            std::cout<<"PASS: fixed-step diagnostic sphere fall, grounding, directional movement and jump\n";
            std::cout<<"PASS: floor/corner sliding, clear sphere movement and near-floor tangent sweep\n";
            std::cout<<"PASS: fast/reverse sphere sweep, clear travel, stationary sphere and initial overlap\n";
            std::cout<<"PASS: sphere face/corner separation, stationary clear sphere and repeat stability\n";
            std::cout<<"PASS: sphere face/edge/vertex contacts, penetration, tangency, metadata and invalid radius\n";
            std::cout<<"PASS: collision nearest segment, metadata, reverse direction, edge, miss and parallel cases\n";
            std::cout<<"PASS: stage table fields, masked values, string bounds, placement SRT and zone composition\n";
            std::cout<<"PASS: BCK interpolation, duplicate timestamps, rotation, animation clock modes and fixed-step cadence\n";
            std::cout << "PASS: WBFS block boundaries, sparse blocks, bounds, malformed headers, mappings and truncation\n";
            std::cout << "PASS: AES/SHA1 known answers, H0 corruption rejection, FST paths/bounds, Yaz0 overlap/truncation, RARC/U8 resources/cycles/bounds\n";
            std::cout << "PASS: 11 GX texture formats, palette/alpha/tiling, TPL, J3D positions/embedded textures and malformed graphics\n";
            std::cout << "PASS: joint hierarchy/transforms, invalid remaps, duplicate joints, unbalanced hierarchy and non-finite transforms\n";
            std::cout << "PASS: mesh primitive winding, triangulation and incomplete primitive rejection\n";
            std::cout << "PASS: joint rotation, parent transforms, Maya scale compensation, weighted skinning and cycle rejection\n";
            std::cout << "PASS: indexed normals/colors/UVs, shape-material mapping, texture references and invalid appearance data\n";
        }
        else if (argc == 3 && std::string(argv[1]) == "--inspect") discInfo(std::filesystem::u8path(argv[2]));
        else if (argc == 4 && std::string(argv[1]) == "--render-test") {
            auto bytes=loadLocal(std::filesystem::u8path(argv[2]));BinaryView view(bytes);
            auto source=view.u32(0)==0x0020af30?decodeTpl(view).at(0):decodeBti(view);
            verifyRendering(source,true);std::cout<<"PASS: Direct3D WARP readback matches "<<source.pixels.size()<<" CPU reference pixels\n";
            auto rendered=verifyRendering(source,false);std::cout<<"PASS: Direct3D hardware readback matches "<<source.pixels.size()<<" CPU reference pixels\n";
            saveNewResource(std::filesystem::u8path(argv[3]),textureBmp(rendered));
        }
        else if (argc == 3 && (std::string(argv[1]) == "--preview-texture"||std::string(argv[1]) == "--preview-smoke")) {
            auto bytes=loadLocal(std::filesystem::u8path(argv[2]));BinaryView view(bytes);
            auto source=view.u32(0)==0x0020af30?decodeTpl(view).at(0):decodeBti(view);
            bool smoke=std::string(argv[1])=="--preview-smoke";previewTexture(source,smoke);
            if(smoke)std::cout<<"PASS: hidden native window swap-chain creation, painting and resize\n";
        }
        else if (argc == 4 && std::string(argv[1]) == "--decode-texture") {
            auto bytes=loadLocal(std::filesystem::u8path(argv[2]));BinaryView view(bytes);
            auto image=view.u32(0)==0x0020af30?decodeTpl(view).at(0):decodeBti(view);
            saveNewResource(std::filesystem::u8path(argv[3]),textureBmp(image));
            std::cout<<"Decoded "<<image.width<<'x'<<image.height<<" GX format "<<image.format<<" texture preview\n";
        }
        else if(argc==6 && std::string(argv[1])=="--native-collision"){
 WiiAssets assets(std::filesystem::u8path(argv[2]));Rarc archive(assets.readFile(argv[3]));auto kcl=archive.readFile(argv[4]),metadata=archive.readFile(argv[5]);auto geometry=decodeCollision(BinaryView(kcl));
 NativeRuntimeClient runtime;runtime.loadTerrain(kcl,metadata,{1,0,0,0,0,1,0,0,0,0,1,0},argv[3]);runtime.tick();size_t samples=0;
 for(const auto& triangle:geometry.triangles){if(samples==32)break;std::array<float,3> position,gravity,movement;
  for(int j=0;j<3;++j){position[j]=(triangle.points[0][j]+triangle.points[1][j]+triangle.points[2][j])/3+triangle.normal[j]*20;gravity[j]=-triangle.normal[j];movement[j]=-triangle.normal[j]*15;}
  auto result=runtime.bind(position,gravity,movement,10);assetCheck(result.contacts!=0,"Original runtime Binder missed terrain");for(float v:result.displacement)assetCheck(std::isfinite(v),"Nonfinite original runtime displacement");++samples;
 }
 assetCheck(samples>0,"No native runtime terrain samples");auto free=runtime.bind({1e7f,1e7f,1e7f},{0,-1,0},{7,-3,5},10);assetCheck(free.contacts==0,"Unexpected distant runtime contact");for(int j=0;j<3;++j)assetCheck(std::abs(free.displacement[j]-std::array<float,3>{7,-3,5}[j])<.001f,"Incorrect free runtime movement");
 std::cout<<"PASS: PC executable loaded original runtime DLL from game assets; "<<samples<<" terrain Binder contacts and free movement; gameplay not yet implemented\n";
}
        else if (argc == 3 && std::string(argv[1]) == "--model") {
            auto bytes=loadLocal(std::filesystem::u8path(argv[2]));auto model=decodeModel(BinaryView(bytes));
            auto rig=decodeRig(BinaryView(bytes));
            for(const auto& [tag,size]:model.sections) std::cout<<tag<<'\t'<<size<<'\n';
            std::cout<<"Decoded "<<model.positions.size()<<" positions, "<<model.textures.size()<<" base textures; header counts: "<<model.joints<<" joints, "<<model.shapes<<" shapes, "<<model.materials<<" materials\n";
            std::cout<<"Rig: "<<rig.joints.size()<<" hierarchy joints, "<<rig.envelopes.size()<<" skin envelopes, "<<rig.drawMatrices.size()<<" draw matrices\n";
        }
        else if(argc==3&&std::string(argv[1])=="--mesh"){
            auto bytes=loadLocal(std::filesystem::u8path(argv[2]));BinaryView view(bytes);auto model=decodeModel(view);auto rig=decodeRig(view);auto mesh=decodeMesh(view,model,rig);
            std::cout<<"Decoded "<<mesh.packets.size()<<" mesh packets, "<<mesh.vertices<<" submitted vertices, "<<mesh.triangles<<" triangles with position/draw-matrix references\n";
        }
        else if(argc==4&&std::string(argv[1])=="--export-bind-obj"){
            auto bytes=loadLocal(std::filesystem::u8path(argv[2]));BinaryView view(bytes);auto model=decodeModel(view);auto rig=decodeRig(view);auto mesh=decodeMesh(view,model,rig);
            saveNewResource(std::filesystem::u8path(argv[3]),exportBindObj(model,rig,mesh));std::cout<<"Exported "<<mesh.triangles<<" default-pose triangles with native skinning\n";
        }
        else if(argc==5&&(std::string(argv[1])=="--verify-stateful-scenario-actors"||std::string(argv[1])=="--verify-moving-scenario-actors"||std::string(argv[1])=="--verify-transforming-scenario-actors")){
            WiiAssets assets(std::filesystem::u8path(argv[2]));size_t consumed=0;auto number=std::stoul(argv[4],&consumed);assetCheck(consumed==std::strlen(argv[4])&&number<=UINT32_MAX,"Invalid scenario number");auto assembly=assembleScenario(assets,argv[3],uint32_t(number));bool transform=std::string(argv[1])=="--verify-transforming-scenario-actors";bool moving=std::string(argv[1])=="--verify-moving-scenario-actors"||transform;auto count=verifyStatefulScenarioActors(assets,assembly,moving,transform);std::cout<<"PASS: "<<count<<" authored native actor adapters; moving="<<moving<<"; transforming="<<transform<<"; initialized, state-scheduled and sensor offsets transformed through original JMap/NameObj/Spine; diagnostic adapters only\n";
        }
        else if(argc==4&&(std::string(argv[1])=="--render-mario-track-blend"||std::string(argv[1])=="--render-mario-group-blend"||std::string(argv[1])=="--render-mario-pose-stages")){
            WiiAssets assets(std::filesystem::u8path(argv[2]));Rarc object(assets.readFile("/ObjectData/Mario.arc")),archive(assets.readFile("/ObjectData/MarioAnime.arc"));auto bytes=object.readFile("/mario.bdl"),waitBytes=archive.readFile("/wait.bck"),runBytes=archive.readFile("/run.bck");BinaryView view(bytes);auto model=decodeModel(view);auto rig=decodeRig(view);auto mesh=decodeMesh(view,model,rig);auto wait=decodeBck(BinaryView(waitBytes)),run=decodeBck(BinaryView(runBytes));auto pose=blendAnimationTracks(rig,{wait.pose(rig,8),run.pose(rig,8)},{1,1});if(std::string(argv[1])=="--render-mario-group-blend"||std::string(argv[1])=="--render-mario-pose-stages"){NativeMarioGroupPlayer player(archive,rig,resolveMarioGroup(doubleAnimeTable[1].mParent.mAnimationName));for(int tick=0;tick<8;++tick)player.tick();pose=player.pose();}if(std::string(argv[1])=="--render-mario-pose-stages")pose=blendAnimationPoseStages(pose,wait.pose(rig,0),run.pose(rig,4),.25f,.5f);auto triangles=materialPoseTriangles(view,model,pose,mesh);auto hardware=renderMesh(triangles,false,720,720,&model.textures),reference=renderMesh(triangles,true,720,720,&model.textures);size_t differences=0,painted=0;for(size_t i=0;i<hardware.pixels.size();++i){auto a=hardware.pixels[i],b=reference.pixels[i];if(std::abs(int(a.r)-b.r)>2||std::abs(int(a.g)-b.g)>2||std::abs(int(a.b)-b.b)>2)++differences;if(a.r!=24||a.g!=28||a.b!=37)++painted;}assetCheck(painted>1000&&differences<hardware.pixels.size()/100,"Quaternion blended Mario render disagreement");saveNewResource(std::filesystem::u8path(argv[3]),textureBmp(hardware));std::cout<<"PASS: Mario quaternion joint blend (original initial weights in group mode); triangles="<<triangles.size()/3<<" painted="<<painted<<" hardware/WARP mismatches="<<differences<<"; native blend adapter only\n";
        }
        else if(argc==4&&(std::string(argv[1])=="--preview-mario-logical-animation"||std::string(argv[1])=="--preview-mario-logical-animation-smoke")){
            WiiAssets assets(std::filesystem::u8path(argv[2]));auto bytes=loadLocal(std::filesystem::u8path(argv[3]));assetCheck(!bytes.empty()&&bytes.size()<=4096,"Invalid logical animation name file");std::string name(bytes.begin(),bytes.end());previewMarioLogicalAnimation(assets,name,std::string(argv[1])=="--preview-mario-logical-animation-smoke");std::cout<<"PASS: game-encoded logical Mario animation selected through original single-clip mapping, authored settings and scheduled animated preview; diagnostic only\n";
        }
        else if(argc==3&&(std::string(argv[1])=="--preview-mario-group"||std::string(argv[1])=="--preview-mario-group-smoke")){
            WiiAssets assets(std::filesystem::u8path(argv[2]));previewMarioGroup(assets,std::string(argv[1])=="--preview-mario-group-smoke");std::cout<<"PASS: actor-scheduled multi-clip Mario preview; diagnostic only\n";
        }
        else if(argc==3&&std::string(argv[1])=="--verify-mario-group-playback"){
            WiiAssets assets(std::filesystem::u8path(argv[2]));Rarc archive(assets.readFile("/ObjectData/MarioAnime.arc")),object(assets.readFile("/ObjectData/Mario.arc"));auto bytes=object.readFile("/mario.bdl");auto rig=decodeRig(BinaryView(bytes));auto count=verifyMarioMultiPlayback(archive,rig);std::cout<<"PASS: "<<count<<" original multi-clip groups, 120 independent controller updates and blended skinning poses each; native playback adapter only\n";
        }
        else if(argc==3&&std::string(argv[1])=="--verify-mario-animation-groups"){
            WiiAssets assets(std::filesystem::u8path(argv[2]));Rarc archive(assets.readFile("/ObjectData/MarioAnime.arc"));auto count=verifyMarioSingleClips(archive);std::cout<<"PASS: "<<count<<" original Mario single-clip mappings resolved and advanced 120 ticks each; multi-clip groups not played\n";
        }
        else if(argc==5&&std::string(argv[1])=="--verify-bck-settings"){
            WiiAssets assets(std::filesystem::u8path(argv[2]));Rarc archive(assets.readFile(argv[3]));auto bytes=archive.readFile(argv[4]);NativeBckSettingsTable settings{BinaryView(bytes)};size_t matched=0;for(const auto& entry:settings.entries){for(const auto& resource:archive.files){auto path=std::filesystem::path(resource.path);if(path.extension()==".bck"&&MR::isEqualStringCase(path.stem().string().c_str(),entry.mName)){auto data=archive.readFile(resource.path);auto animation=decodeBck(BinaryView(data));AnimationClock clock(animation);applyBckSettings(clock,settings.select(entry.mName));for(int i=0;i<120;++i)clock.tick();++matched;break;}}}assetCheck(matched>0,"No authored settings matched BCK clips");std::cout<<"PASS: "<<settings.entries.size()<<" authored clip settings; "<<matched<<" matching BCK clips configured and advanced 120 ticks each\n";
        }
        else if(argc==5&&std::string(argv[1])=="--verify-scenario-actors"){
            WiiAssets assets(std::filesystem::u8path(argv[2]));size_t consumed=0;auto number=std::stoul(argv[4],&consumed);assetCheck(consumed==std::strlen(argv[4])&&number<=UINT32_MAX,"Invalid scenario number");auto assembly=assembleScenario(assets,argv[3],uint32_t(number));NativeScenarioActors runtime(assets,assembly);
            for(int tick=0;tick<3;++tick)runtime.tick();for(auto actor:runtime.actors)assetCheck(actor->updates==3&&actor->afterPlacement,"Scenario actor movement/lifecycle mismatch");
            if(!runtime.actors.empty()){auto first=runtime.actors.front();first->requestSuspend();runtime.tick();assetCheck(first->updates==3,"Scenario actor suspension failed");first->requestResume();runtime.tick();assetCheck(first->updates==4,"Scenario actor resume failed");for(size_t i=1;i<runtime.actors.size();++i)assetCheck(runtime.actors[i]->updates==5,"Scenario actor suspension affected peer");}
            std::cout<<"PASS: "<<runtime.actors.size()<<" active objinfo adapters across "<<assembly.zones<<" zone instances; original reader initialization, world transforms, movement, suspend/resume verified; "<<assembly.disabledTables<<" disabled tables excluded; native adapters only\n";
        }
        else if(argc==4&&std::string(argv[1])=="--verify-placed-actors"){
            WiiAssets assets(std::filesystem::u8path(argv[2]));Rarc archive(assets.readFile(argv[3]));NativeScene scene;size_t count=0;
            for(const auto& resource:archive.files)if(resource.path.rfind("/jmp/placement/",0)==0&&std::filesystem::path(resource.path).filename()=="objinfo"){
                auto bytes=archive.readFile(resource.path);StageTable checked{BinaryView(bytes)};auto data=std::make_shared<NativeJmapTable>(BinaryView(bytes));
                for(const auto& record:decodePlacements(resource.path,checked)){auto& actor=scene.place<NativePlacedActor>(JMapInfoIter(&data->info,int(record.row)),data,identityMatrix());auto expected=placementMatrix(record);assetCheck(actor.world==expected&&actor.placement.name==record.name&&actor.afterPlacement,"Original placement reader/lifecycle mismatch");++count;}
            }scene.tick();assetCheck(scene.size()==count,"Placed actor ownership count mismatch");std::cout<<"PASS: "<<count<<" actual placement records initialized through original NameObj/JMapInfo, transforms matched, owned scene movement dispatched; native adapters only\n";
        }
        else if(argc==5&&std::string(argv[1])=="--scenario-starts"){
            WiiAssets assets(std::filesystem::u8path(argv[2]));size_t consumed=0;auto number=std::stoul(argv[4],&consumed);assetCheck(consumed==std::strlen(argv[4])&&number<=UINT32_MAX,"Invalid scenario number");auto assembly=assembleScenario(assets,argv[3],uint32_t(number));size_t count=0;for(const auto& record:assembly.placements)if(record.startId){std::cout<<"START zone="<<record.zone<<" instance="<<record.zoneInstance<<" MarioNo="<<*record.startId<<" name="<<record.placement.name<<" position="<<record.world[3]<<','<<record.world[7]<<','<<record.world[11]<<'\n';++count;}std::cout<<"PASS: "<<count<<" active starts with stored IDs and parent world transforms\n";
        }
        else if(argc==3&&std::string(argv[1])=="--verify-segment-gravity"){
            WiiAssets assets(std::filesystem::u8path(argv[2]));size_t archives=0,records=0,queries=0;
            for(const auto& file:assets.files){auto path=std::filesystem::path(file.path);if(path.parent_path()!=std::filesystem::path("/StageData")||path.extension()!=".arc")continue;Rarc archive(assets.readFile(file.path));++archives;
                for(const auto& resource:archive.files)if(std::filesystem::path(resource.path).filename()=="planetobjinfo"){auto bytes=archive.readFile(resource.path);StageTable table{BinaryView(bytes)};table.validate();for(const auto& placement:decodePlacements(resource.path,table))if(placement.name=="GlobalSegmentGravity"){
                    auto segment=decodeSegmentGravity(placement,table);auto probe=segment;probe.parameters.range=-1;probe.parameters.inverse=false;probe.sectorCos=-1;probe.baseEdge=probe.endEdge=true;
                    CollisionPoint side{},base{},end{};for(size_t k=0;k<3;++k){side[k]=(segment.parameters.center[k]+segment.end[k])*.5f+segment.sectorAxis[k]*100;base[k]=segment.parameters.center[k]-segment.axis[k]*100;end[k]=segment.end[k]+segment.axis[k]*100;}
                    for(auto sample:{side,base,end}){auto direction=probe.direction(sample);auto distance=probe.distance(sample);assetCheck(direction&&distance&&std::abs(*distance-100)<1,"Retail segment transform/distance check failed");assetCheck(std::abs(collisionDot(*direction,*direction)-1)<.001,"Retail segment direction not normalized");std::vector<GravitySource> sources{{probe,5,true,"retail segment"}};auto selection=selectGravity(sources,sample);assetCheck(selection&&selection->priority==5&&collisionDot(selection->direction,*direction)>.999,"Retail segment manager selection failed");++queries;}
                    if(records<8)std::cout<<"VERIFIED "<<file.path<<':'<<resource.path<<" row="<<placement.row<<" length="<<segment.length<<'\n';++records;
                }}
            }assetCheck(records>0,"No retail segment placements found");std::cout<<"PASS: "<<archives<<" stage archives scanned, "<<records<<" stored segment placements decoded, "<<queries<<" geometric/selection queries; all stored layers, not scenario activation\n";
        }
        else if(argc==4&&std::string(argv[1])=="--verify-native-jmap"){
            WiiAssets assets(std::filesystem::u8path(argv[2]));Rarc archive(assets.readFile(argv[3]));size_t tables=0,cells=0;
            for(const auto& resource:archive.files)if(resource.path.rfind("/jmp/",0)==0){try{auto bytes=archive.readFile(resource.path);cells+=verifyNativeJmap(BinaryView(bytes));++tables;}catch(const std::exception& e){throw std::runtime_error(resource.path+": "+e.what());}}
            std::cout<<"PASS: "<<tables<<" actual stage tables, "<<cells<<" cells agree between checked parser and original JMapInfo via aligned host-endian bridge\n";
        }
        else if(argc==7&&std::string(argv[1])=="--render-scenario-mario"){
            WiiAssets assets(std::filesystem::u8path(argv[2]));size_t consumed=0;auto number=std::stoul(argv[4],&consumed);assetCheck(consumed==std::strlen(argv[4])&&number<=UINT32_MAX,"Invalid scenario number");auto assembly=assembleScenario(assets,argv[3],uint32_t(number));auto collision=loadScenarioZoneCollision(assets,assembly,argv[5]);auto field=loadScenarioZoneGravity(assets,assembly,argv[5]);auto scene=loadScenarioZoneScene(assets,assembly,argv[5]);NativeScenarioActors actors(assets,assembly);DiagnosticMarioModel mario(assets);DiagnosticBody body;bool found=false;
            CollisionPoint initialFacing{};if(auto start=findScenarioStart(assembly,argv[5],0)){assetCheck(start->placement.name=="Mario","Snapshot start actor is unsupported");body.position={start->world[3],start->world[7],start->world[11]};initialFacing={start->world[2],start->world[6],start->world[10]};found=true;}assetCheck(found,"No active MarioNo 0 start for snapshot");
            auto gravity=gravityBodyVector(field,body.position);for(size_t k=0;k<3;++k)body.position[k]-=gravity[k]/100*body.radius;CollisionIndex index(collision.geometry);CollisionPoint up{};
            for(int tick=0;tick<120;++tick){actors.tick();gravity=gravityBodyVector(field,body.position);for(size_t k=0;k<3;++k)up[k]=-gravity[k]/100;body.tick(index,gravity,{0,0,0});mario.tick(false,body.grounded,collisionDot(body.velocity,up));}assetCheck(body.partialSteps==0,"Snapshot diagnostic body had incomplete collision steps");actors.renderPhases();
            CollisionPoint axis=std::abs(up[1])<.9f?CollisionPoint{0,1,0}:CollisionPoint{1,0,0};auto right=collisionCross(axis,up);float length=std::sqrt(collisionDot(right,right));for(float& value:right)value/=length;auto forward=collisionCross(up,right);
            DiagnosticHeading heading;heading.update(up,initialFacing,forward);auto visual=mario.triangles(body.position,up,heading.right,heading.forward,body.radius,int(scene.textures.size()));scene.triangles.insert(scene.triangles.end(),visual.begin(),visual.end());scene.textures.insert(scene.textures.end(),mario.textures().begin(),mario.textures().end());MeshCameraBounds camera;std::vector<RenderVertex> corners;for(int x:{-1,1})for(int y:{-1,1})for(int z:{-1,1})corners.push_back({{body.position[0]+x*80,body.position[1]+y*80,body.position[2]+z*80},{0,1,0}});camera.include(corners);
            auto reference=renderMesh(scene.triangles,true,720,720,&scene.textures,nullptr,&camera),hardware=renderMesh(scene.triangles,false,720,720,&scene.textures,nullptr,&camera);size_t differences=0;for(size_t i=0;i<hardware.pixels.size();++i){auto a=hardware.pixels[i],b=reference.pixels[i];if(std::abs(int(a.r)-b.r)>2||std::abs(int(a.g)-b.g)>2||std::abs(int(a.b)-b.b)>2)++differences;}assetCheck(differences<hardware.pixels.size()/100,"Scenario Mario snapshot hardware/WARP disagreement");saveNewResource(std::filesystem::u8path(argv[6]),textureBmp(hardware));std::cout<<"PASS: diagnostic Mario snapshot after 120 updates; grounded="<<body.grounded<<" visual triangles="<<visual.size()/3<<" active adapters="<<actors.actors.size()<<" hardware/WARP mismatches="<<differences<<"; not original Mario gameplay\n";
        }
        else if(argc==3&&std::string(argv[1])=="--verify-player-save"){
            auto path=std::filesystem::u8path(argv[2]);assetCheck(!std::filesystem::exists(path),"Save verification requires a new file");NativePlayerStock stock;stock.addPlayerLeft(95);writePlayerStatusFile(path,stock);NativePlayerStock loaded;assetCheck(readPlayerStatusFile(path,loaded)&&loaded.getPlayerLeft()==4,"Disk player status load mismatch");stock.addPlayerLeft(-99);writePlayerStatusFile(path,stock);assetCheck(readPlayerStatusFile(path,loaded)&&loaded.getPlayerLeft()==4,"Atomic player save replacement mismatch");auto bad=stock.saveBytes();bad.pop_back();loaded.addPlayerLeft(1);assetCheck(!loaded.loadBytes(bad)&&loaded.getPlayerLeft()==5,"Invalid saved data mutated live status");std::cout<<"PASS: Wii player-status disk write/load/replacement and rejected corrupt payload; original session-life reset preserved\n";
        }
        else if(argc==3&&std::string(argv[1])=="--verify-campaign-save"){
            WiiAssets assets(std::filesystem::u8path(argv[2]));auto catalog=loadCampaignCatalog(assets);NativeCampaignProgress source,restored;initializeCampaignCatalog(source,catalog);initializeCampaignCatalog(restored,catalog);size_t tracked=0,stars=0;
            for(const auto& descriptor:catalog){stars+=descriptor.stars;if(!descriptor.stars)continue;++tracked;auto progress=source.galaxy(descriptor.name,int(descriptor.stars));auto scenario=progress->scenario(1);scenario.setPowerStarFlag(true);scenario.setFlagAlreadyVisited(true);scenario.updateMaxCoinNum(321);}
            auto bytes=encodeSaveChunks({&source});assetCheck(decodeSaveChunks(bytes,{&restored}),"Disc campaign save restore rejected");
            for(const auto& descriptor:catalog)if(descriptor.stars){auto scenario=restored.galaxy(descriptor.name,int(descriptor.stars))->scenario(1);assetCheck(scenario.hasPowerStar()&&scenario.isAlreadyVisited()&&scenario.getMaxCoinNum()==321,"Disc campaign record lost progress");}
            std::cout<<"PASS: disc campaign archives="<<catalog.size()<<" tracked galaxies="<<tracked<<" authored stars="<<stars<<" GALA save bytes="<<bytes.size()<<"; explicit test flags, not campaign gameplay\n";
        }
        else if(argc==4&&std::string(argv[1])=="--render-miss-text"){
            WiiAssets assets(std::filesystem::u8path(argv[2]));auto resources=loadNativeMissUi(assets);size_t visible=0;for(float time:{0.f,18.f,30.f,60.f}){auto pose=sampleLayoutPose(resources.layout,resources.animations.at("Appear"),time);auto mesh=layoutTextQuads(pose,resources.fonts);assetCheck(mesh.size()==14,"Missing authored miss/shadow glyphs");TextureImage image{608,456,6,std::vector<Pixel>(608*456,{80,110,140,255})};auto painted=compositeLayoutText(image,pose,resources.fonts);if(time==0)assetCheck(painted==0,"Initial miss parent alpha did not hide text");visible+=painted;auto path=std::filesystem::u8path(argv[3]);path+="-"+std::to_string(int(time))+".bmp";saveNewResource(path,textureBmp(image));std::cout<<"frame="<<time<<" glyph quads="<<mesh.size()<<" composed pixels="<<painted<<"\n";}assetCheck(visible>1000,"Miss text remained invisible");std::cout<<"PASS: authored font metrics, animated miss text/shadow gradients and composition; native adapter, retail visual parity pending\n";
        }
        else if(argc==4&&std::string(argv[1])=="--render-miss-font-glyphs"){
            WiiAssets assets(std::filesystem::u8path(argv[2]));Rarc archive(assets.readFile("/LayoutData/Font.arc"));auto bytes=archive.readFile("/menufont64.brfnt");auto font=decodeResourceFont(BinaryView(bytes));TextureImage image{8u*font.cellWidth,font.cellHeight,6,std::vector<Pixel>(8u*font.cellWidth*font.cellHeight,{0,0,0,255})};std::u16string text=u"TOO BAD!";size_t ink=0;for(size_t i=0;i<text.size();++i){auto index=font.glyphIndex(text[i]);auto cell=font.glyphCell(index);auto widths=font.glyphWidths(index);const auto& sheet=font.sheets.at(cell[0]);assetCheck(cell[1]+widths.width<=sheet.width&&cell[2]+font.cellHeight<=sheet.height,"Font glyph crop outside sheet");for(unsigned y=0;y<font.cellHeight;++y)for(unsigned x=0;x<widths.width;++x){auto pixel=sheet.pixels[size_t(cell[2]+y)*sheet.width+cell[1]+x];image.pixels[size_t(y)*image.width+i*font.cellWidth+x]=pixel;if(pixel.a)++ink;}std::cout<<char(text[i])<<" glyph="<<index<<" width="<<unsigned(widths.width)<<" advance="<<int(widths.advance)<<"\n";}assetCheck(ink>100,"Empty font glyph preview");saveNewResource(std::filesystem::u8path(argv[3]),textureBmp(image));auto truncated=bytes;truncated.pop_back();bool rejected=false;try{decodeResourceFont(BinaryView(truncated));}catch(const std::exception&){rejected=true;}assetCheck(rejected,"Truncated font accepted");std::cout<<"PASS: retail font sheets="<<font.sheets.size()<<" maps="<<font.maps.size()<<" ink="<<ink<<"; native decoder/glyph preview, animated text composition pending\n";
        }
        else if(argc==3&&std::string(argv[1])=="--verify-miss-english-message"){
            WiiAssets assets(std::filesystem::u8path(argv[2]));Rarc messages(assets.readFile("/UsEnglish/MessageData/Message.arc"));NativeMessageCatalog catalog(messages);Rarc archive(assets.readFile("/LayoutData/TooBad.arc"));auto bytes=archive.readFile("/blyt/toobad.brlyt");auto layout=decodeLayoutResource(BinaryView(bytes));auto bindings=layoutTextMessageBindings(layout,"TooBad");assetCheck(bindings.size()==2,"Missing miss text bindings");for(const auto& binding:bindings){auto text=catalog.plainText(binding.second);assetCheck(!text.empty()&&text!=u"DOMMAGE!","English miss message retained placeholder");layout.panes[binding.first].text->text=text;std::cout<<binding.second<<" text=";for(auto unit:text){assetCheck(unit<128,"English miss text unexpectedly non-ASCII");std::cout<<char(unit);}std::cout<<"\n";}auto runtimeResources=loadNativeMissUi(assets);assetCheck(runtimeResources.fonts.size()==1&&runtimeResources.fonts[0].glyphIndex(uint16_t(u"T"[0]))==52,"Runtime miss font mapping mismatch");for(float frame:{0.f,30.f,61.f}){auto pose=sampleLayoutPose(runtimeResources.layout,runtimeResources.animations.at("Appear"),frame);for(const auto& pane:pose.panes)if(pane.text)assetCheck(pane.text->text==u"TOO BAD!","Runtime miss pose lost English binding");}std::cout<<"PASS: retail English message catalog IDs="<<catalog.size()<<" and authored miss bindings; glyph rendering pending\n";
        }
        else if(argc==3&&std::string(argv[1])=="--verify-miss-text-layout"){
            WiiAssets assets(std::filesystem::u8path(argv[2]));Rarc archive(assets.readFile("/LayoutData/TooBad.arc"));auto bytes=archive.readFile("/blyt/toobad.brlyt");auto layout=decodeLayoutResource(BinaryView(bytes));assetCheck(layout.fontNames==std::vector<std::string>{"MenuFont64.brfnt"},"Unexpected authored miss font");size_t texts=0;for(const auto& pane:layout.panes)if(pane.text){const auto& text=*pane.text;assetCheck(text.text==u"DOMMAGE!"&&text.fontSize==std::array<float,2>{85,85}&&text.bufferBytes==18&&text.stringBytes==18&&text.position==4,"Authored miss text fields mismatch");++texts;std::cout<<pane.name<<" font="<<layout.fontNames[text.font]<<" glyphs="<<text.text.size()<<" material="<<pane.material<<"\n";}assetCheck(texts==2,"Unexpected miss text pane count");auto bindings=layoutTextMessageBindings(layout,"TooBad");assetCheck(bindings.size()==2&&bindings[0].second=="Layout_TooBadTooBad"&&bindings[1].second==bindings[0].second,"Authored inherited miss message binding mismatch");std::cout<<"localized message="<<bindings[0].second<<"\n";for(const char* name:{"appear","end","wait"}){auto data=archive.readFile(std::string("/anim/")+name+".brlan");auto animation=decodeLayoutAnimation(BinaryView(data));auto pose=sampleLayoutPose(layout,animation,0);assetCheck(pose.panes.size()==layout.panes.size(),"Miss animation changed pane count");std::cout<<name<<" tracks="<<animation.tracks.size()<<"\n";}Rarc fonts(assets.readFile("/LayoutData/Font.arc"));bool found=false;for(const auto& file:fonts.files){std::string path=file.path;std::transform(path.begin(),path.end(),path.begin(),[](unsigned char c){return char(std::tolower(c));});if(path.find("menufont64.brfnt")!=std::string::npos){found=true;std::cout<<"font resource="<<file.path<<" bytes="<<file.size<<"\n";}}assetCheck(found,"Missing authored miss font resource");std::cout<<"PASS: authored text/font references and pane animations; placeholder text retained, localized message binding/glyph rendering pending\n";
        }
        else if(argc==4&&std::string(argv[1])=="--render-wipe-composition"){
            WiiAssets assets(std::filesystem::u8path(argv[2]));Rarc archive(assets.readFile("/LayoutData/WipeKoopa.arc"));auto resources=loadNativeLayoutAssets(archive,"/blyt/wipekoopa.brlyt");auto bytes=archive.readFile("/anim/out.brlan");auto animation=decodeLayoutAnimation(BinaryView(bytes));
            for(float time:{0.f,45.f,90.f}){auto pose=sampleLayoutPose(resources.layout,animation,time);TextureImage frame{608,456,6,std::vector<Pixel>(608*456,{200,160,100,255})};auto painted=compositeLayoutWindows(frame,pose,resources.textures);if(time==0)assetCheck(painted==0,"Open wipe altered frame");if(time==90){assetCheck(painted==frame.pixels.size(),"Closed wipe did not cover viewport");for(auto pixel:frame.pixels)assetCheck(pixel.r==0&&pixel.g==0&&pixel.b==0,"Closed wipe not black");}auto path=std::filesystem::u8path(argv[3]);path+="-"+std::to_string(int(time))+".bmp";saveNewResource(path,textureBmp(frame));std::cout<<"frame="<<time<<" composed pixels="<<painted<<"\n";}
            std::cout<<"PASS: authored animated wipe composed into native frame; native CPU adapter, retail viewport parity pending\n";
        }
        else if(argc==3&&std::string(argv[1])=="--verify-wipe-texture-sampling"){
            WiiAssets assets(std::filesystem::u8path(argv[2]));Rarc archive(assets.readFile("/LayoutData/WipeKoopa.arc"));auto resources=loadNativeLayoutAssets(archive,"/blyt/wipekoopa.brlyt");size_t samples=0;
            for(const auto& material:resources.layout.materials){const auto& map=material.textures.at(0);const auto& texture=resources.textures.at(map.texture);for(uint32_t y=0;y<texture.height;++y)for(uint32_t x=0;x<texture.width;++x){auto sample=sampleLayoutTexture(texture,map,{(x+.5f)/texture.width,(y+.5f)/texture.height});const auto& pixel=texture.pixels[size_t(y)*texture.width+x];assetCheck(std::abs(sample[3]-pixel.a)<.01f,"Authored layout texel-center sampling mismatch");auto color=shadeLayoutTexture(material,sample,0xFFFFFFFF,148);assetCheck(color[0]==0&&color[1]==0&&color[2]==0&&color[3]>=0&&color[3]<=148,"Authored sampled wipe shading mismatch");++samples;}std::cout<<material.name<<" wrap="<<unsigned(map.wrapS)<<","<<unsigned(map.wrapT)<<" filters="<<unsigned(texture.minFilter)<<","<<unsigned(texture.magFilter)<<"\n";}
            std::cout<<"PASS: authored layout sampler settings, texel centers and sampled shading; samples="<<samples<<"; visible rendering pending\n";
        }
        else if(argc==3&&std::string(argv[1])=="--verify-wipe-material-shading"){
            WiiAssets assets(std::filesystem::u8path(argv[2]));Rarc archive(assets.readFile("/LayoutData/WipeKoopa.arc"));auto resources=loadNativeLayoutAssets(archive,"/blyt/wipekoopa.brlyt");size_t samples=0;
            for(const auto& material:resources.layout.materials){assetCheck(material.tevColors[0]==std::array<int16_t,4>{0,0,0,0}&&material.tevColors[1]==std::array<int16_t,4>{0,0,0,255},"Unexpected authored wipe TEV colors");const auto& texture=resources.textures.at(material.textures.at(0).texture);float minimum=255,maximum=0;for(const auto& pixel:texture.pixels){auto color=shadeLayoutTexel(material,pixel,0xFFFFFFFF,148);assetCheck(color[0]==0&&color[1]==0&&color[2]==0&&std::abs(color[3]-float(pixel.a)*148/255)<.0001f,"Authored wipe mask shading mismatch");minimum=std::min(minimum,color[3]);maximum=std::max(maximum,color[3]);++samples;}std::cout<<material.name<<" alpha range="<<minimum<<".."<<maximum<<"\n";}
            std::cout<<"PASS: authored wipe material black/alpha shading, pixels="<<samples<<"; floating-point adapter, visible rendering pending\n";
        }
        else if(argc==3&&std::string(argv[1])=="--verify-wipe-texture-transforms"){
            WiiAssets assets(std::filesystem::u8path(argv[2]));Rarc archive(assets.readFile("/LayoutData/WipeKoopa.arc"));auto resources=loadNativeLayoutAssets(archive,"/blyt/wipekoopa.brlyt");auto bytes=archive.readFile("/anim/out.brlan");auto animation=decodeLayoutAnimation(BinaryView(bytes));auto pose=sampleLayoutPose(resources.layout,animation,45);auto original=layoutWindowQuads(pose,resources.textures);auto transformed=transformLayoutTextureCoordinates(original,pose);assetCheck(transformed.size()==5,"Missing wipe texture quads");assetCheck(transformed.front().uv==std::array<float,8>{0,0,2,0,0,1,2,1},"Authored wipe content UV transform mismatch");for(size_t i=1;i<transformed.size();++i)assetCheck(transformed[i].uv==original[i].uv,"Authored blank frame UV changed");std::cout<<"PASS: authored content UV scale/translation, identity frame UVs and stored TEV colors; native coordinate adapter, shading/rendering pending\n";
        }
        else if(argc==3&&std::string(argv[1])=="--verify-wipe-window-mesh"){
            WiiAssets assets(std::filesystem::u8path(argv[2]));Rarc archive(assets.readFile("/LayoutData/WipeKoopa.arc"));auto resources=loadNativeLayoutAssets(archive,"/blyt/wipekoopa.brlyt");auto bytes=archive.readFile("/anim/out.brlan");auto animation=decodeLayoutAnimation(BinaryView(bytes));
            for(float frame:{0.0f,45.0f,90.0f}){auto pose=sampleLayoutPose(resources.layout,animation,frame);auto mesh=layoutWindowQuads(pose,resources.textures);assetCheck(mesh.size()==(frame==90?4:5),"Authored wipe window mesh count mismatch");double area=0;for(const auto& quad:mesh)area+=double(quad.positions[1][0]-quad.positions[0][0])*(quad.positions[0][1]-quad.positions[2][1]);auto pane=std::find_if(pose.panes.begin(),pose.panes.end(),[](const auto& p){return p.name=="WinKoopa";});assetCheck(pane!=pose.panes.end()&&std::abs(area-double(pane->size[0])*pane->size[1]*256*204)<32,"Authored window quad tiling mismatch");std::cout<<"frame="<<frame<<" quads="<<mesh.size()<<" area="<<area<<"\n";}
            std::cout<<"PASS: authored animated single-frame wipe window mesh; native adapter, shading/rendering pending\n";
        }
        else if(argc==3&&std::string(argv[1])=="--verify-wipe-world-geometry"){
            WiiAssets assets(std::filesystem::u8path(argv[2]));Rarc archive(assets.readFile("/LayoutData/WipeKoopa.arc"));auto resources=loadNativeLayoutAssets(archive,"/blyt/wipekoopa.brlyt");auto bytes=archive.readFile("/anim/out.brlan");auto animation=decodeLayoutAnimation(BinaryView(bytes));
            for(float frame:{0.0f,45.0f,90.0f}){auto pose=sampleLayoutPose(resources.layout,animation,frame);auto world=layoutWorldPanes(pose);size_t index=0;while(index<pose.panes.size()&&pose.panes[index].name!="WinKoopa")++index;assetCheck(index<pose.panes.size(),"Missing authored wipe geometry");auto width=world[index].corners[1][0]-world[index].corners[0][0],height=world[index].corners[0][1]-world[index].corners[2][1];assetCheck(std::abs(width-pose.panes[index].size[0]*256)<0.01f&&std::abs(height-pose.panes[index].size[1]*204)<0.01f&&world[index].visible,"Authored hierarchical wipe geometry mismatch");std::cout<<"frame="<<frame<<" world bounds="<<width<<"x"<<height<<" alpha="<<world[index].alpha<<"\n";}
            std::cout<<"PASS: authored animated wipe hierarchy transformed in layout coordinates; viewport/window rendering pending\n";
        }
        else if(argc==3&&std::string(argv[1])=="--verify-wipe-pane-animation"){
            WiiAssets assets(std::filesystem::u8path(argv[2]));Rarc archive(assets.readFile("/LayoutData/WipeKoopa.arc"));auto resources=loadNativeLayoutAssets(archive,"/blyt/wipekoopa.brlyt");auto bytes=archive.readFile("/anim/out.brlan");auto animation=decodeLayoutAnimation(BinaryView(bytes));
            for(float frame:{0.0f,45.0f,90.0f}){auto pose=sampleLayoutPose(resources.layout,animation,frame);auto found=std::find_if(pose.panes.begin(),pose.panes.end(),[](const auto& pane){return pane.name=="WinKoopa";});assetCheck(found!=pose.panes.end(),"Missing authored animated wipe window");assetCheck(found->size[0]==found->size[1]&&found->alpha==uint8_t(frame==0?0:frame==90?255:148),"Authored wipe pane size/alpha mismatch");if(frame==0||frame==90)assetCheck(found->size[0]==(frame==0?30:16),"Authored wipe size endpoint mismatch");std::cout<<"frame="<<frame<<" window="<<found->size[0]<<"x"<<found->size[1]<<" alpha="<<unsigned(found->alpha)<<"\n";}
            std::cout<<"PASS: authored wipe tracks applied to immutable pane poses; native adapter, rendering pending\n";
        }
        else if(argc==3&&std::string(argv[1])=="--verify-wipe-animation-tracks"){
            WiiAssets assets(std::filesystem::u8path(argv[2]));Rarc archive(assets.readFile("/LayoutData/WipeKoopa.arc"));size_t total=0;
            for(const char* name:{"in","out"}){auto bytes=archive.readFile(std::string("/anim/")+name+".brlan");auto animation=decodeLayoutAnimation(BinaryView(bytes));assetCheck(!animation.tracks.empty(),"Empty authored wipe animation");for(const auto& track:animation.tracks){std::cout<<name<<" "<<track.name<<" "<<track.kind<<" target="<<int(track.target)<<" keys="<<track.keys.size()<<" samples="<<sampleLayoutTrack(track,0)<<","<<sampleLayoutTrack(track,45)<<","<<sampleLayoutTrack(track,90)<<"\n";for(unsigned frame=0;frame<=animation.metadata.frames;++frame)sampleLayoutTrack(track,float(frame));++total;}}
            assetCheck(total==6,"Unexpected authored wipe animation track count");std::cout<<"PASS: authored BRLAN tracks decoded and sampled; native curve evaluator, pane application/rendering pending\n";
        }
        else if(argc==3&&std::string(argv[1])=="--verify-down-wipe-resources"){
            WiiAssets assets(std::filesystem::u8path(argv[2]));Rarc archive(assets.readFile("/LayoutData/WipeKoopa.arc"));auto resources=loadNativeLayoutAssets(archive,"/blyt/wipekoopa.brlyt");const auto& layout=resources.layout;size_t pictures=0,textures=0,pixels=0;
            for(const auto& pane:layout.panes){std::cout<<pane.kind<<" "<<pane.name<<" parent="<<pane.parent<<" size="<<pane.size[0]<<"x"<<pane.size[1]<<" uv="<<pane.uv.size()<<"\n";if(pane.kind=="pic1"||pane.kind=="wnd1")++pictures;}
            for(const auto& file:archive.files)if(std::filesystem::path(file.path).extension()==".tpl"){auto resource=archive.readFile(file.path);auto images=decodeTpl(BinaryView(resource));for(const auto& image:images){std::cout<<file.path<<" "<<image.width<<"x"<<image.height<<" format="<<image.format<<"\n";pixels+=image.pixels.size();++textures;}}
            for(const auto& material:layout.materials)for(const auto& map:material.textures)std::cout<<"MATERIAL "<<material.name<<" -> "<<layout.textureNames.at(map.texture)<<" "<<resources.textures.at(map.texture).width<<"x"<<resources.textures.at(map.texture).height<<" SRT="<<material.textureSrt.size()<<"\n";assetCheck(pictures>0&&textures==2&&pixels>0&&resources.textures.size()==2&&layout.materials.size()==2,"Missing authored wipe resources");std::cout<<"PASS: authored wipe pane hierarchy/picture data and decoded textures; material texture references resolved; TEV/animation/rendering pending\n";
        }
        else if(argc==3&&std::string(argv[1])=="--verify-miss-animation"){
            WiiAssets assets(std::filesystem::u8path(argv[2]));Rarc archive(assets.readFile("/LayoutData/TooBad.arc"));size_t count=0;
            for(const auto& resource:archive.files)if(std::filesystem::path(resource.path).extension()==".brlan"){auto bytes=archive.readFile(resource.path);auto metadata=decodeLayoutAnimationMetadata(BinaryView(bytes));std::cout<<resource.path<<" frames="<<metadata.frames<<" loop="<<metadata.loop<<" contents="<<metadata.contents<<"\n";++count;}
            assetCheck(count==3,"Unexpected TooBad animation resource count");std::cout<<"PASS: authored miss-layout animation timing metadata; pane tracks/rendering not implemented\n";
        }
        else if(argc==5&&std::string(argv[1])=="--verify-scenario-death-sequence"){
            WiiAssets assets(std::filesystem::u8path(argv[2]));size_t consumed=0;auto number=std::stoul(argv[4],&consumed);assetCheck(consumed==std::strlen(argv[4])&&number<=UINT32_MAX,"Invalid scenario number");auto assembly=assembleScenario(assets,argv[3],uint32_t(number));NativeScenarioActors runtime(assets,assembly);runtime.tick();bool started=false;
            for(size_t i=0;i<runtime.fatalAreaCount();++i){auto point=runtime.fatalArea(i).interior();if(runtime.beginFatalSequenceAt(point)){assetCheck(!runtime.beginFatalSequenceAt(point),"Duplicate authored death sequence accepted");started=true;break;}}assetCheck(started,"No authored fatal area could start abyss sequence");TextureImage wipeFrame{608,456,6,std::vector<Pixel>(608*456,{200,160,100,255})};assetCheck(runtime.composeClosingWipe(wipeFrame)==0,"Death sequence displayed wipe before request");
            for(int i=0;i<120;++i){runtime.tick();if(i==29){TextureImage missFrame{608,456,6,std::vector<Pixel>(608*456,{80,110,140,255})};auto painted=runtime.composeDeathUi(missFrame);size_t red=0;for(auto pixel:missFrame.pixels)if(int(pixel.r)>int(pixel.g)+30&&int(pixel.r)>int(pixel.b)+30)++red;assetCheck(painted>1000&&red>1000,"Live original miss actor did not render localized red text");}}assetCheck(runtime.playerLives()==4,"Authored abyss decremented lives before frame 120");runtime.tick();auto& sequence=runtime.playerDeathSequence();assetCheck(runtime.playerLives()==3&&sequence.services.lifeDecrements==1&&sequence.running(),"Authored abyss frame-120 life event mismatch");
            assetCheck(!runtime.missLayoutActor().isActive()&&runtime.missLayoutActor().nativeAnimStopped&&sequence.services.downWipes==1&&sequence.running(),"Authored animation did not close original miss layout and gate wipe");sequence.services.wipeActive=false;runtime.tick();assetCheck(sequence.running()&&sequence.services.endMiss==0,"Scenario death bypassed bound wipe state");for(int i=0;i<256&&sequence.running();++i)runtime.tick();assetCheck(!sequence.running()&&sequence.services.endMiss==1&&runtime.playerLives()==3,"Authored abyss automatic wipe/sequence end mismatch");assetCheck(runtime.composeClosingWipe(wipeFrame)==wipeFrame.pixels.size(),"Runtime closing wipe did not cover frame at completion");for(auto pixel:wipeFrame.pixels)assetCheck(pixel.r==0&&pixel.g==0&&pixel.b==0,"Runtime completed wipe not black");
            std::cout<<"PASS: authored fatal area -> original abyss scheduler -> frame-120 life decrement -> gated miss completion; original miss-layout lifecycle, authored animation and original wipe completion, no verifier completion signals; native miss text/shadow and closing wipe composed; Mario death/scene retry unported\n";
        }
        else if(argc==5&&std::string(argv[1])=="--verify-scenario-death-areas"){
            WiiAssets assets(std::filesystem::u8path(argv[2]));size_t consumed=0;auto number=std::stoul(argv[4],&consumed);assetCheck(consumed==std::strlen(argv[4])&&number<=UINT32_MAX,"Invalid scenario number");auto assembly=assembleScenario(assets,argv[3],uint32_t(number));NativeScenarioActors runtime(assets,assembly);runtime.tick();assetCheck(runtime.fatalAreaCount()>0,"Scenario has no supported fatal areas");size_t active=0;
            for(size_t i=0;i<runtime.fatalAreaCount();++i){const auto& area=runtime.fatalArea(i);bool expected=area.isValid()&&area.getDeathType()==DeathArea::DeathType_Any&&(!area.isValidSwitchA()||area.isOnSwitchA());auto point=area.interior();assetCheck(runtime.evaluateFatalArea(i,point)==expected,"Authored fatal-area predicate mismatch");if(expected){++active;assetCheck(runtime.requestFatalAreaAt(point),"Authored abyss event not delivered");}}
            assetCheck(active>0,"Scenario has no active player-fatal areas");std::cout<<"PASS: authored fatal areas="<<runtime.fatalAreaCount()<<" active player-fatal interiors="<<active<<"; original DeathArea predicate, native geometry/events; original Mario death unported\n";
        }
        else if(argc==5&&std::string(argv[1])=="--verify-scenario-checkpoints"){
            WiiAssets assets(std::filesystem::u8path(argv[2]));size_t consumed=0;auto number=std::stoul(argv[4],&consumed);assetCheck(consumed==std::strlen(argv[4])&&number<=UINT32_MAX,"Invalid scenario number");auto assembly=assembleScenario(assets,argv[3],uint32_t(number));NativeScenarioActors runtime(assets,assembly);runtime.tick();assetCheck(runtime.restartCubeCount()>0,"Scenario has no authored restart cubes");size_t verified=0;
            for(size_t i=0;i<runtime.restartCubeCount();++i){const auto& cube=runtime.restartCube(i);if(!cube.isValid())continue;auto point=cube.interior();runtime.temporaryStageMemory().resetRestart();assetCheck(runtime.updatePlayerCheckpoint(point,true),"Authored restart area rejected interior");const NativeRestartCube* selected=nullptr;for(size_t j=runtime.restartCubeCount();j>0;--j)if(runtime.restartCube(j-1).isInVolume({point[0],point[1],point[2]})){selected=&runtime.restartCube(j-1);break;}assetCheck(selected&&runtime.temporaryStageMemory().restartInfo()==*selected->mIdInfo,"Authored restart trigger set wrong start/zone");auto target=runtime.restartPlacement(assembly);assetCheck(target&&target->placement.name=="Mario","Authored checkpoint has no supported target start");++verified;}
            assetCheck(verified>0,"Scenario has no active restart cubes");std::cout<<"PASS: authored restart cubes="<<runtime.restartCubeCount()<<" active interiors="<<verified<<"; authored targets resolved, original trigger/native volumes; original retry gameplay unported\n";
        }
        else if(argc==6&&std::string(argv[1])=="--verify-scenario-coins"){
            WiiAssets assets(std::filesystem::u8path(argv[2]));size_t consumed=0;auto number=std::stoul(argv[4],&consumed);assetCheck(consumed==std::strlen(argv[4])&&number<=UINT32_MAX,"Invalid scenario number");auto assembly=assembleScenario(assets,argv[3],uint32_t(number));auto scene=loadScenarioZoneScene(assets,assembly,argv[5]);assetCheck(!scene.coins.empty(),"Selected scenario zone has no supported Coin/PurpleCoin geometry");NativeScene owner;auto& rotation=owner.create<NativeCoinRotater>();owner.tick();auto posed=stageSceneWithCoinRotation(scene,rotation.normal);size_t changed=0,coinVertices=0;for(const auto& coin:scene.coins)for(size_t i=0;i<coin.local.size();++i){auto index=coin.offset+i;++coinVertices;assetCheck(posed[index].texture==scene.triangles[index].texture&&posed[index].uv==scene.triangles[index].uv&&posed[index].color==scene.triangles[index].color,"Authored coin appearance changed during rotation");if(posed[index].position!=scene.triangles[index].position)++changed;}assetCheck(changed>0,"Authored coin geometry did not rotate");NativeCampaignProgress campaign;NativeScenarioActors actorRuntime(assets,assembly,&campaign);assetCheck(!actorRuntime.actors.empty(),"No authored stage flag actor");uint32_t done=99;auto doneIndex=actorRuntime.setupActorDoneFlag(0,"native verification",done);assetCheck(done==0,"Authored stage flag initial value mismatch");actorRuntime.temporaryStageMemory().update(doneIndex,1);assetCheck(actorRuntime.setupActorDoneFlag(0,"native verification",done)==doneIndex&&done==1,"Authored stage flag did not retain event");assetCheck(actorRuntime.scenarioVisited()&&!actorRuntime.scenarioStarOwned(),"Authored progress initial state mismatch");actorRuntime.recordPowerStarCollected();assetCheck(actorRuntime.scenarioStarOwned()&&actorRuntime.collectedStageResult().isStageCleared()&&actorRuntime.collectedStageResult().mPowerStarId==int(number)&&std::string(actorRuntime.collectedStageResult().mStageName)==argv[3],"Authored star completion/result not recorded");bool collected=false;for(const auto& coin:scene.coins){if(!actorRuntime.coinVisible(coin.actorKey))continue;int before=actorRuntime.coinCount()+actorRuntime.purpleCoinCount();assetCheck(actorRuntime.tryCollectCoin(coin.actorKey),"Authored coin sensor item-get rejected");assetCheck(!actorRuntime.coinVisible(coin.actorKey)&&actorRuntime.coinCount()+actorRuntime.purpleCoinCount()==before+1,"Authored coin sensor reward/hide mismatch");assetCheck(!actorRuntime.tryCollectCoin(coin.actorKey)&&actorRuntime.coinCount()+actorRuntime.purpleCoinCount()==before+1,"Authored coin sensor awarded twice");collected=true;break;}assetCheck(collected,"No visible authored coin to verify sensor collection");if(assembly.powerStarCount>1){int alternate=number==1?2:1;actorRuntime.recordPowerStarCollected(alternate);assetCheck(actorRuntime.collectedStageResult().mPowerStarId==alternate&&actorRuntime.collectedStageResult().mCoinNum==actorRuntime.coinCount()&&campaign.galaxy(assembly.galaxy,int(assembly.powerStarCount))->scenario(alternate).hasPowerStar(),"Collected alternate star ID/result routing mismatch");}NativeScenarioActors contacts(assets,assembly,&campaign);assetCheck(contacts.scenarioVisited()&&contacts.scenarioStarOwned(),"Second authored runtime lost session progress");bool contacted=false;for(const auto& coin:scene.coins){if(!contacts.coinVisible(coin.actorKey))continue;auto position=contacts.coinSensorPosition(coin.actorKey);int before=contacts.coinCount()+contacts.purpleCoinCount();contacts.collectCoinsAt(position,1);assetCheck(!contacts.coinVisible(coin.actorKey)&&contacts.coinCount()+contacts.purpleCoinCount()>before,"Coin contact did not collect");int after=contacts.coinCount()+contacts.purpleCoinCount();contacts.collectCoinsAt(position,1);assetCheck(contacts.coinCount()+contacts.purpleCoinCount()==after,"Repeated contact awarded again");contacted=true;break;}assetCheck(contacted,"No visible coin for authored contact verification");std::cout<<"PASS: authored rotating coins="<<scene.coins.size()<<" vertices="<<coinVertices<<" changed="<<changed<<"; texture/UV/color preserved, native visual policy only\n";
        }
        else if((argc==6||argc==7)&&(std::string(argv[1])=="--verify-scenario-body"||std::string(argv[1])=="--preview-scenario-body"||std::string(argv[1])=="--preview-scenario-body-smoke"||std::string(argv[1])=="--preview-scenario-mario"||std::string(argv[1])=="--preview-scenario-mario-smoke")){
            WiiAssets assets(std::filesystem::u8path(argv[2]));size_t consumed=0;auto number=std::stoul(argv[4],&consumed);assetCheck(consumed==std::strlen(argv[4])&&number<=UINT32_MAX,"Invalid scenario number");auto assembly=assembleScenario(assets,argv[3],uint32_t(number));auto collision=loadScenarioZoneCollision(assets,assembly,"");auto field=loadScenarioZoneGravity(assets,assembly,"");std::optional<CollisionPoint> spawn,facing;
            int32_t startId=0;if(argc==7){size_t used=0;auto value=std::stoll(argv[6],&used);assetCheck(used==std::strlen(argv[6])&&value>=INT32_MIN&&value<=INT32_MAX,"Invalid MarioNo start ID");startId=int32_t(value);}if(auto start=findScenarioStart(assembly,argv[5],startId)){assetCheck(start->placement.name=="Mario","Diagnostic start actor is unsupported");spawn=CollisionPoint{start->world[3],start->world[7],start->world[11]};facing=CollisionPoint{start->world[2],start->world[6],start->world[10]};}
            assetCheck(spawn.has_value(),"No active Mario start in selected diagnostic zone");NativeScenarioActors scenarioActors(assets,assembly);
            if(std::string(argv[1])=="--verify-scenario-body"){
                CollisionIndex index(collision.geometry);DiagnosticBody body;body.position=*spawn;auto gravity=gravityBodyVector(field,body.position);for(size_t k=0;k<3;++k)body.position[k]-=gravity[k]/100*body.radius;
                for(int i=0;i<120;++i){scenarioActors.tick();body.tick(index,gravityBodyVector(field,body.position),{0,0,0});}for(auto actor:scenarioActors.actors)assetCheck(actor->updates==120,"Scenario actor/body cadence mismatch");double residual=0;for(auto contact:sphereContacts(collision.geometry,body.position,body.radius))residual=std::max(residual,contact.penetration);
                assetCheck(body.partialSteps==0&&residual<=.001,"Scenario body collision check failed");std::cout<<"PASS: 120 scenario-world diagnostic body updates; grounded="<<body.grounded<<" partial="<<body.partialSteps<<" residual="<<residual<<" collision instances="<<collision.instances.size()<<" gravity sources="<<field.sources.size()<<'\n';
            }else{auto scene=loadScenarioZoneScene(assets,assembly,"");auto command=std::string(argv[1]);bool smoke=command=="--preview-scenario-body-smoke"||command=="--preview-scenario-mario-smoke";std::unique_ptr<DiagnosticMarioModel> mario;if(command=="--preview-scenario-mario"||command=="--preview-scenario-mario-smoke")mario=std::make_unique<DiagnosticMarioModel>(assets);previewDiagnosticBody(scene,collision,smoke,&field,&*spawn,&scenarioActors,mario.get(),&*facing,[&]()->std::optional<AffineMatrix>{auto target=scenarioActors.restartPlacement(assembly);assetCheck(target&&target->placement.name=="Mario","Checkpoint target start unavailable");return target->world;});if(smoke)std::cout<<"PASS: whole-scenario visual/collision/gravity with selected start and diagnostic checkpoint retry, "<<scenarioActors.actors.size()<<" active actor adapters updated alongside body; Mario visual="<<bool(mario)<<" (diagnostic control only)\n";}
        }
        else if(argc==6&&std::string(argv[1])=="--scenario-gravity"){
            WiiAssets assets(std::filesystem::u8path(argv[2]));size_t consumed=0;auto number=std::stoul(argv[4],&consumed);assetCheck(consumed==std::strlen(argv[4])&&number<=UINT32_MAX,"Invalid scenario number");auto assembly=assembleScenario(assets,argv[3],uint32_t(number));auto field=loadScenarioZoneGravity(assets,assembly,argv[5]);size_t starts=0,covered=0;
            for(const auto& record:assembly.placements)if(record.zone==argv[5]&&std::filesystem::path(record.placement.table).filename()=="startinfo"){CollisionPoint point{record.world[3],record.world[7],record.world[11]};auto gravity=selectGravity(field.sources,point);++starts;if(gravity)++covered;std::cout<<"START row="<<record.placement.row<<" gravity="<<bool(gravity);if(gravity)std::cout<<" priority="<<gravity->priority<<" direction="<<gravity->direction[0]<<','<<gravity->direction[1]<<','<<gravity->direction[2];std::cout<<'\n';}
            std::cout<<"PASS: scenario world gravity sources="<<field.sources.size()<<" unsupported="<<field.unsupported<<" starts="<<starts<<" covered="<<covered<<'\n';
        }
        else if(argc==6&&std::string(argv[1])=="--verify-scenario-collision"){
            WiiAssets assets(std::filesystem::u8path(argv[2]));size_t consumed=0;auto number=std::stoul(argv[4],&consumed);assetCheck(consumed==std::strlen(argv[4])&&number<=UINT32_MAX,"Invalid scenario number");auto assembly=assembleScenario(assets,argv[3],uint32_t(number));auto collision=loadScenarioZoneCollision(assets,assembly,argv[5]);
            for(const auto& item:collision.instances)std::cout<<"LOADED "<<item<<'\n';for(const auto& item:collision.skipped)std::cout<<"SKIPPED "<<item<<'\n';
            auto segments=verifyCollisionQueries(collision.geometry),spheres=verifySphereContacts(collision.geometry),candidates=verifyCollisionIndex(collision.geometry);
            std::cout<<"PASS: scenario collision instances="<<collision.instances.size()<<" triangles="<<collision.geometry.triangles.size()<<" skipped="<<collision.skipped.size()<<" segment checks="<<segments<<" sphere checks="<<spheres<<" indexed candidates="<<candidates<<"; active layers/parent transforms\n";
        }
        else if(argc==7&&std::string(argv[1])=="--render-scenario-zone"){
            WiiAssets assets(std::filesystem::u8path(argv[2]));size_t consumed=0;auto number=std::stoul(argv[4],&consumed);assetCheck(consumed==std::strlen(argv[4])&&number<=UINT32_MAX,"Invalid scenario number");auto assembly=assembleScenario(assets,argv[3],uint32_t(number));auto scene=loadScenarioZoneScene(assets,assembly,argv[5]);
            for(const auto& item:scene.loaded)std::cout<<"LOADED "<<item<<'\n';for(const auto& item:scene.skipped)std::cout<<"SKIPPED "<<item<<'\n';
            auto hardware=renderMesh(scene.triangles,false,960,720,&scene.textures),reference=renderMesh(scene.triangles,true,960,720,&scene.textures);size_t differences=0;
            for(size_t i=0;i<hardware.pixels.size();++i){auto a=hardware.pixels[i],b=reference.pixels[i];if(std::abs(int(a.r)-b.r)>2||std::abs(int(a.g)-b.g)>2||std::abs(int(a.b)-b.b)>2)++differences;}
            assetCheck(differences<hardware.pixels.size()/100,"Scenario zone render disagreement");saveNewResource(std::filesystem::u8path(argv[6]),textureBmp(hardware));std::cout<<"PASS: scenario-zone models="<<scene.loaded.size()<<" unique prototypes="<<scene.uniqueModels<<" texture entries="<<scene.textures.size()<<" skipped="<<scene.skipped.size()<<" triangles="<<scene.triangles.size()/3<<" hardware/WARP mismatches="<<differences<<"; active layers with parent world transforms\n";
        }
        else if(argc==5&&std::string(argv[1])=="--scenario-placement-tree"){
            WiiAssets assets(std::filesystem::u8path(argv[2]));size_t consumed=0;auto number=std::stoul(argv[4],&consumed);assetCheck(consumed==std::strlen(argv[4])&&number<=UINT32_MAX,"Invalid scenario number");auto assembly=assembleScenario(assets,argv[3],uint32_t(number));
            std::cout<<"zone\tinstance\ttable\trow\tname\tworld_x\tworld_y\tworld_z\n";for(const auto& record:assembly.placements)std::cout<<record.zone<<'\t'<<record.zoneInstance<<'\t'<<record.placement.table<<'\t'<<record.placement.row<<'\t'<<record.placement.name<<'\t'<<record.world[3]<<'\t'<<record.world[7]<<'\t'<<record.world[11]<<'\n';
            std::cerr<<"PASS: scenario zones="<<assembly.zones<<" active placement records="<<assembly.placements.size()<<" disabled tables="<<assembly.disabledTables<<"; world matrix composition complete, actor/render integration pending\n";
        }
        else if(argc==5&&std::string(argv[1])=="--scenario-layers"){
            WiiAssets assets(std::filesystem::u8path(argv[2]));size_t consumed=0;auto scenario=std::stoul(argv[4],&consumed);assetCheck(consumed==std::strlen(argv[4])&&scenario<=UINT32_MAX,"Invalid scenario number");auto zones=loadScenarioLayers(assets,argv[3],uint32_t(scenario));
            for(const auto& zone:zones){std::cout<<zone.name<<" mask="<<zone.layers<<" enabled=common";for(char letter='a';letter<='p';++letter)if(scenarioLayerEnabled(std::string("layer")+letter,zone.layers))std::cout<<','<<letter;std::cout<<'\n';}std::cout<<"PASS: "<<zones.size()<<" zone layer masks for scenario "<<scenario<<"; scene filtering not connected\n";
        }
        else if(argc==4&&std::string(argv[1])=="--verify-gravity-body"){
            WiiAssets assets(std::filesystem::u8path(argv[2]));auto collision=loadCommonCollision(assets,argv[3]);auto field=loadCommonGravity(assets,argv[3]);CollisionIndex index(collision.geometry);DiagnosticBody body;body.position=loadCommonPlayerStart(assets,argv[3]);auto gravity=gravityBodyVector(field,body.position);for(size_t k=0;k<3;++k)body.position[k]-=gravity[k]/100*body.radius;
            for(int i=0;i<120;++i)body.tick(index,gravityBodyVector(field,body.position),{0,0,0});double residual=0;for(auto contact:sphereContacts(collision.geometry,body.position,body.radius))residual=std::max(residual,contact.penetration);
            assetCheck(body.partialSteps==0&&residual<=.001,"Stored-start gravity body collision failed");std::cout<<"PASS: 120 diagnostic body updates from stored Mario start using supported gravity; grounded="<<body.grounded<<" partial="<<body.partialSteps<<" residual="<<residual<<'\n';
        }
        else if(argc==4&&(std::string(argv[1])=="--preview-gravity-body"||std::string(argv[1])=="--preview-gravity-body-smoke")){
            WiiAssets assets(std::filesystem::u8path(argv[2]));auto scene=loadCommonScene(assets,argv[3]);auto collision=loadCommonCollision(assets,argv[3]);auto field=loadCommonGravity(assets,argv[3]);auto spawn=loadCommonPlayerStart(assets,argv[3]);bool smoke=std::string(argv[1])=="--preview-gravity-body-smoke";
            previewDiagnosticBody(scene,collision,smoke,&field,&spawn);if(smoke)std::cout<<"PASS: stored player start and supported gravity field integrated into diagnostic body window\n";
        }
        else if(argc==4&&std::string(argv[1])=="--zone-gravity"){
            WiiAssets assets(std::filesystem::u8path(argv[2]));auto field=loadCommonGravity(assets,argv[3]);Rarc archive(assets.readFile(argv[3]));size_t sampled=0,covered=0;
            for(const auto& resource:archive.files)if(resource.path=="/jmp/start/common/startinfo"){auto bytes=archive.readFile(resource.path);StageTable table{BinaryView(bytes)};for(const auto& placement:decodePlacements(resource.path,table)){auto gravity=selectGravity(field.sources,placement.position);++sampled;if(gravity)++covered;std::cout<<"START row="<<placement.row<<" gravity="<<bool(gravity);if(gravity)std::cout<<" priority="<<gravity->priority<<" sources="<<gravity->contributing<<" direction="<<gravity->direction[0]<<','<<gravity->direction[1]<<','<<gravity->direction[2];std::cout<<'\n';}}
            std::cout<<"PASS: supported sources="<<field.sources.size()<<" unsupported="<<field.unsupported<<" player starts sampled="<<sampled<<" covered="<<covered<<"; local common-layer field only\n";
        }
        else if(argc==4&&std::string(argv[1])=="--zone-disk-gravity"){
            WiiAssets assets(std::filesystem::u8path(argv[2]));Rarc archive(assets.readFile(argv[3]));size_t disks=0,unsupported=0;
            for(const auto& resource:archive.files)if(resource.path=="/jmp/placement/common/planetobjinfo"){auto bytes=archive.readFile(resource.path);StageTable table{BinaryView(bytes)};
                for(const auto& placement:decodePlacements(resource.path,table)){if(placement.name!="GlobalDiskGravity"){++unsupported;continue;}auto disk=decodeDiskGravity(placement,table);auto sample=disk.parameters.center;for(size_t k=0;k<3;++k)sample[k]+=disk.normal[k];disk.direction(sample);++disks;std::cout<<"DISK row="<<placement.row<<" radius="<<disk.radius<<" range="<<disk.parameters.range<<" bothSides="<<disk.bothSides<<" edge="<<disk.edgeGravity<<'\n';}}
            std::cout<<"PASS: disk sources="<<disks<<" unsupported gravity records="<<unsupported<<"; common layer/local coordinates, manager blending pending\n";
        }
        else if(argc==4&&std::string(argv[1])=="--zone-point-gravity"){
            WiiAssets assets(std::filesystem::u8path(argv[2]));Rarc archive(assets.readFile(argv[3]));size_t points=0,unsupported=0;
            for(const auto& resource:archive.files)if(resource.path=="/jmp/placement/common/planetobjinfo"){
                auto bytes=archive.readFile(resource.path);StageTable table{BinaryView(bytes)};
                for(const auto& placement:decodePlacements(resource.path,table)){if(placement.name!="GlobalPointGravity"){++unsupported;continue;}auto gravity=decodePointGravity(placement,table);auto sample=gravity.center;sample[0]+=1;gravity.direction(sample);++points;
                    std::cout<<"POINT row="<<placement.row<<" center="<<gravity.center[0]<<','<<gravity.center[1]<<','<<gravity.center[2]<<" range="<<gravity.range<<" distant="<<gravity.distant<<" inverse="<<gravity.inverse<<'\n';}
            }std::cout<<"PASS: point sources="<<points<<" unsupported gravity records="<<unsupported<<"; common layer, local zone only, no manager blending\n";
        }
        else if(argc==4&&(std::string(argv[1])=="--preview-runtime-body"||std::string(argv[1])=="--preview-runtime-body-smoke")){
 WiiAssets assets(std::filesystem::u8path(argv[2]));auto scene=loadCommonScene(assets,argv[3]);auto collision=loadCommonCollision(assets,argv[3],true);NativeRuntimeClient runtime;
 for(const auto& terrain:collision.runtimeTerrain)runtime.loadTerrain(terrain.kcl,terrain.metadata,terrain.matrix,terrain.name);
 assetCheck(collision.runtimeTerrain.size()==collision.instances.size(),"Original runtime terrain coverage mismatch");bool smoke=std::string(argv[1])=="--preview-runtime-body-smoke";
 previewDiagnosticBody(scene,collision,smoke,nullptr,nullptr,nullptr,nullptr,nullptr,{},&runtime);
 std::cout<<"PASS: rendered diagnostic body used original runtime Binder for "<<runtime.bindCalls<<" updates on "<<collision.runtimeTerrain.size()<<" placed terrain instances; original Mario controller not implemented\n";
}
        else if(argc==4&&(std::string(argv[1])=="--preview-zone-body"||std::string(argv[1])=="--preview-zone-body-smoke")){
            WiiAssets assets(std::filesystem::u8path(argv[2]));auto scene=loadCommonScene(assets,argv[3]);auto collision=loadCommonCollision(assets,argv[3]);bool smoke=std::string(argv[1])=="--preview-zone-body-smoke";
            previewDiagnosticBody(scene,collision,smoke);if(smoke)std::cout<<"PASS: diagnostic body fixed updates, scene rendering, key input/focus loss and window lifecycle\n";
        }
        else if(argc==4&&std::string(argv[1])=="--verify-zone-body"){
            WiiAssets assets(std::filesystem::u8path(argv[2]));auto collision=loadCommonCollision(assets,argv[3]);auto count=verifyDiagnosticBody(collision.geometry);
            std::cout<<"PASS: "<<count<<" diagnostic sphere fall/grounding runs of 120 fixed updates on placed zone geometry; no partial steps or residual overlaps\n";
        }
        else if(argc==4&&std::string(argv[1])=="--verify-zone-collision"){
            WiiAssets assets(std::filesystem::u8path(argv[2]));auto collision=loadCommonCollision(assets,argv[3]);
            for(const auto& item:collision.instances)std::cout<<"LOADED "<<item<<'\n';for(const auto& item:collision.skipped)std::cout<<"SKIPPED "<<item<<'\n';
            auto count=verifyCollisionQueries(collision.geometry);auto spheres=verifySphereContacts(collision.geometry);auto candidates=verifyCollisionIndex(collision.geometry);auto separation=verifySphereSeparation(collision.geometry);auto sweeps=verifySphereSweeps(collision.geometry);auto movement=verifySphereMovement(collision.geometry);std::cout<<"PASS: real placed-geometry sphere sweeps="<<sweeps<<" movement complete="<<movement.first<<" partial="<<movement.second<<'\n';
            std::cout<<"PASS: placed collision instances="<<collision.instances.size()<<" triangles="<<collision.geometry.triangles.size()<<" skipped="<<collision.skipped.size()<<" segment checks="<<count<<" sphere checks="<<spheres<<" indexed candidate triangles="<<candidates<<" full-scan equivalent="<<(count+spheres)*collision.geometry.triangles.size()<<" separation resolved="<<separation.first<<" unresolved="<<separation.second<<"; common layer, local zone coordinates\n";
        }
        else if(argc==3&&std::string(argv[1])=="--verify-collision"){
            auto bytes=loadLocal(std::filesystem::u8path(argv[2]));auto geometry=decodeCollision(BinaryView(bytes));auto count=verifyCollisionQueries(geometry);
            std::cout<<"PASS: "<<geometry.triangles.size()<<" reconstructed KCL triangles; "<<count<<" real-geometry isolated/full-mesh segment checks; octree and player response pending\n";
        }
        else if(argc==4&&std::string(argv[1])=="--render-collision"){
            auto bytes=loadLocal(std::filesystem::u8path(argv[2]));auto geometry=decodeCollision(BinaryView(bytes));std::vector<RenderVertex> triangles;
            for(const auto& triangle:geometry.triangles)for(auto p:triangle.points)triangles.push_back({p,triangle.normal});
            auto hardware=renderMesh(triangles,false),reference=renderMesh(triangles,true);size_t differences=0;
            for(size_t i=0;i<hardware.pixels.size();++i){auto a=hardware.pixels[i],b=reference.pixels[i];if(std::abs(int(a.r)-b.r)>2||std::abs(int(a.g)-b.g)>2||std::abs(int(a.b)-b.b)>2)++differences;}
            assetCheck(differences<hardware.pixels.size()/100,"Collision rendering comparison failed");saveNewResource(std::filesystem::u8path(argv[3]),textureBmp(hardware));
            std::cout<<"PASS: KCL prisms="<<geometry.prisms<<" triangles="<<geometry.triangles.size()<<" degenerate/skipped="<<geometry.degenerate<<" hardware/WARP mismatches="<<differences<<"; octree not traversed\n";
        }
        else if((argc==5&&std::string(argv[1])=="--render-zone")||(argc==4&&std::string(argv[1])=="--preview-zone")){
            WiiAssets assets(std::filesystem::u8path(argv[2]));auto scene=loadCommonScene(assets,argv[3]);
            for(const auto& loaded:scene.loaded)std::cout<<"LOADED "<<loaded<<'\n';for(const auto& skipped:scene.skipped)std::cout<<"SKIPPED "<<skipped<<'\n';
            auto hardware=renderMesh(scene.triangles,false,960,720,&scene.textures),reference=renderMesh(scene.triangles,true,960,720,&scene.textures);size_t differences=0,painted=0;
            for(size_t i=0;i<hardware.pixels.size();++i){auto a=hardware.pixels[i],b=reference.pixels[i];if(std::abs(int(a.r)-b.r)>2||std::abs(int(a.g)-b.g)>2||std::abs(int(a.b)-b.b)>2)++differences;if(a.r!=24||a.g!=28||a.b!=37)++painted;}
            assetCheck(painted>1000&&differences<hardware.pixels.size()/100,"Zone scene rendering comparison failed");
            if(argc==5)saveNewResource(std::filesystem::u8path(argv[4]),textureBmp(hardware));else previewTexture(hardware);
            std::cout<<"PASS: common-layer scene instances="<<scene.loaded.size()<<" unique prototypes="<<scene.uniqueModels<<" texture entries="<<scene.textures.size()<<" skipped="<<scene.skipped.size()<<" triangles="<<scene.triangles.size()/3<<" textures="<<scene.textures.size()<<" geometry pixels="<<painted<<" hardware/WARP mismatches="<<differences<<'\n';
        }
        else if((argc==7||argc==6||argc==8)&&(std::string(argv[1])=="--preview-stateful-animation"||std::string(argv[1])=="--preview-stateful-animation-smoke"||std::string(argv[1])=="--preview-stateful-placement"||std::string(argv[1])=="--preview-stateful-placement-smoke"||std::string(argv[1])=="--render-placement"||std::string(argv[1])=="--render-stateful-placement")){
            WiiAssets assets(std::filesystem::u8path(argv[2]));Rarc stage(assets.readFile(argv[3]));auto tableBytes=stage.readFile(argv[4]);StageTable table{BinaryView(tableBytes)};
            size_t consumed=0;unsigned long row=std::stoul(argv[5],&consumed);assetCheck(consumed==std::strlen(argv[5]),"Invalid placement row");
            auto placements=decodePlacements(argv[4],table);assetCheck(row<placements.size(),"Placement row out of range");auto placement=placements[row];
            Rarc object(assets.readFile("/ObjectData/"+placement.name+".arc"));std::vector<std::string> candidates;
            for(const auto& resource:object.files){auto ext=std::filesystem::path(resource.path).extension().string();if(ext==".bdl"||ext==".bmd")candidates.push_back(resource.path);}
            assetCheck(candidates.size()==1,"Placement requires explicit model selection: archive does not have exactly one model");
            auto modelBytes=object.readFile(candidates[0]);BinaryView view(modelBytes);auto model=decodeModel(view);auto rig=decodeRig(view);auto mesh=decodeMesh(view,model,rig);auto triangles=materialPoseTriangles(view,model,rig,mesh);
            if(std::string(argv[1]).rfind("--preview-stateful-animation",0)==0){assetCheck(argc==8,"Missing animation archive/resource arguments");Rarc animationArchive(assets.readFile(argv[6]));auto animationBytes=animationArchive.readFile(argv[7]);auto animation=decodeBck(BinaryView(animationBytes));std::unique_ptr<NativeBckSettingsTable> settings;auto desired=std::filesystem::path(argv[6]).stem().string()+".banmt";for(const auto& resource:animationArchive.files)if(MR::isEqualStringCase(std::filesystem::path(resource.path).filename().string().c_str(),desired.c_str())){auto data=animationArchive.readFile(resource.path);settings=std::make_unique<NativeBckSettingsTable>(BinaryView(data));break;}auto clipName=std::filesystem::path(argv[7]).stem().string();previewStatefulPlacement(std::make_shared<NativeJmapTable>(BinaryView(tableBytes)),row,view,model,rig,mesh,std::string(argv[1])=="--preview-stateful-animation-smoke",&animation,settings?&settings->select(clipName.c_str()):nullptr);std::cout<<"PASS: actor-state-driven joint animation, model transforms and display lifecycle; diagnostic only\n";return 0;}
            if(std::string(argv[1]).rfind("--preview-stateful-placement",0)==0){assetCheck(argc==6,"Unexpected preview output argument");previewStatefulPlacement(std::make_shared<NativeJmapTable>(BinaryView(tableBytes)),row,view,model,rig,mesh,std::string(argv[1])=="--preview-stateful-placement-smoke");std::cout<<"PASS: authored actor state-driven textured preview and display lifecycle; diagnostic only\n";return 0;}assetCheck(argc==7,"Missing render output path");
            bool stateful=std::string(argv[1])=="--render-stateful-placement";auto matrix=stateful?advanceDiagnosticPlacement(std::make_shared<NativeJmapTable>(BinaryView(tableBytes)),row):placementMatrix(placement);for(auto& vertex:triangles){vertex.position=transformPoint(matrix,vertex.position);vertex.normal=skinNormal(matrix,vertex.normal,vertex.normal);}
            auto hardware=renderMesh(triangles,false,720,720,&model.textures),reference=renderMesh(triangles,true,720,720,&model.textures);size_t differences=0,painted=0;
            for(size_t i=0;i<hardware.pixels.size();++i){auto a=hardware.pixels[i],b=reference.pixels[i];if(std::abs(int(a.r)-b.r)>2||std::abs(int(a.g)-b.g)>2||std::abs(int(a.b)-b.b)>2)++differences;if(a.r!=24||a.g!=28||a.b!=37)++painted;}
            assetCheck(painted>1000&&differences<hardware.pixels.size()/100,"Placed model rendering comparison failed");saveNewResource(std::filesystem::u8path(argv[6]),textureBmp(hardware));
            std::cout<<"PASS: placed "<<placement.name<<" model="<<candidates[0]<<" triangles="<<triangles.size()/3<<" geometry pixels="<<painted<<" hardware/WARP mismatches="<<differences<<"; stateful="<<stateful<<"; local-zone diagnostic transform only\n";
        }
        else if(argc==4&&std::string(argv[1])=="--stage-placements"){
            WiiAssets assets(std::filesystem::u8path(argv[2]));Rarc archive(assets.readFile(argv[3]));size_t total=0,matched=0,zones=0;
            std::cout<<"table\trow\tname\tpos_x\tpos_y\tpos_z\tdir_x\tdir_y\tdir_z\tscale_x\tscale_y\tscale_z\tdirect_archive\n";
            for(const auto& resource:archive.files){auto leaf=std::filesystem::path(resource.path).filename().string();
                if(leaf!="objinfo"&&leaf!="planetobjinfo"&&leaf!="stageobjinfo"&&leaf!="startinfo")continue;
                auto bytes=archive.readFile(resource.path);StageTable table{BinaryView(bytes)};table.validate();
                for(const auto& p:decodePlacements(resource.path,table)){placementMatrix(p);++total;bool zone=leaf=="stageobjinfo";if(zone)++zones;
                    std::string candidate=(zone?"/StageData/":"/ObjectData/")+p.name+".arc",reference;
                    for(const auto& file:assets.files)if(file.path==candidate){reference=candidate;++matched;break;}
                    std::string name=p.name;for(char& c:name)if(c=='\t'||c=='\n'||c=='\r')c=' ';
                    std::cout<<p.table<<'\t'<<p.row<<'\t'<<name;for(float v:p.position)std::cout<<'\t'<<v;for(float v:p.rotationDegrees)std::cout<<'\t'<<v;for(float v:p.scale)std::cout<<'\t'<<v;std::cout<<'\t'<<reference<<'\n';
                }
            }std::cerr<<"PASS: "<<total<<" placements, "<<zones<<" zone records, "<<matched<<" direct-name archive matches (not actor resource resolution)\n";
        }
        else if(argc==4&&std::string(argv[1])=="--stage-tables"){
            WiiAssets assets(std::filesystem::u8path(argv[2]));Rarc archive(assets.readFile(argv[3]));size_t tables=0,rows=0;
            std::cout<<"resource\trows\tfields\n";
            for(const auto& resource:archive.files)if(resource.path.rfind("/jmp/",0)==0){try{auto bytes=archive.readFile(resource.path);StageTable table{BinaryView(bytes)};table.validate();++tables;rows+=table.rows;
                std::cout<<resource.path<<'\t'<<table.rows<<'\t'<<table.fields.size()<<'\n';
            }catch(const std::exception& e){throw std::runtime_error(resource.path+": "+e.what());}}
            std::cerr<<"PASS: "<<tables<<" stage tables, "<<rows<<" records; all cells checked\n";
        }
        else if(argc==4&&(std::string(argv[1])=="--preview-animation"||std::string(argv[1])=="--preview-animation-smoke")){
            auto bytes=loadLocal(std::filesystem::u8path(argv[2])),animationBytes=loadLocal(std::filesystem::u8path(argv[3]));BinaryView view(bytes);
            auto model=decodeModel(view);auto rig=decodeRig(view);auto mesh=decodeMesh(view,model,rig);auto animation=decodeBck(BinaryView(animationBytes));
            bool smoke=std::string(argv[1])=="--preview-animation-smoke";previewAnimation(view,model,rig,mesh,animation,smoke);
            if(smoke)std::cout<<"PASS: eight live skinned poses, retained GPU device/textures, display updates, pause, resize and close\n";
        }
        else if(argc==4&&std::string(argv[1])=="--animation-timeline"){
            auto bytes=loadLocal(std::filesystem::u8path(argv[2]));auto animation=decodeBck(BinaryView(bytes));AnimationClock clock(animation);
            size_t consumed=0;unsigned long ticks=std::stoul(argv[3],&consumed);assetCheck(consumed==std::strlen(argv[3])&&ticks<=36000,"Invalid tick count");
            std::cout<<"tick\tframe\trate\tstate\n";for(size_t i=0;i<=ticks;++i){std::cout<<i<<'\t'<<clock.frame<<'\t'<<clock.rate<<'\t'<<int(clock.state)<<'\n';if(i<ticks)clock.tick();}
        }
        else if(argc==3&&std::string(argv[1])=="--verify-animations"){
            WiiAssets assets(std::filesystem::u8path(argv[2]));size_t archives=0,animations=0,joints=0,keys=0,samples=0;
            std::cout<<"archive\tresource\tjoints\tduration\tloop\n";
            for(const auto& file:assets.files)if(std::filesystem::path(file.path).extension()==".arc"){
                Rarc archive(assets.readFile(file.path));++archives;
                for(const auto& resource:archive.files)if(std::filesystem::path(resource.path).extension()==".bck"){
                    try{auto bytes=archive.readFile(resource.path);auto animation=decodeBck(BinaryView(bytes));++animations;joints+=animation.joints.size();
                        for(const auto& joint:animation.joints)for(const auto& track:joint){keys+=track.keys.size();
                            for(float frame:{0.0f,animation.duration*.5f,animation.duration}){assetCheck(std::isfinite(track.sample(frame,0)),"Invalid sampled animation");++samples;}}
                        std::cout<<file.path<<'\t'<<resource.path<<'\t'<<animation.joints.size()<<'\t'<<animation.duration<<'\t'<<int(animation.loop)<<'\n';
                    }catch(const std::exception& e){throw std::runtime_error(file.path+":"+resource.path+": "+e.what());}
                }
                if(archives%200==0)std::cerr<<"Checked animation resources in "<<archives<<" archives\n";
            }
            std::cerr<<"PASS: "<<animations<<" BCK animations, "<<joints<<" joint tracks, "<<keys<<" channel keys, "<<samples<<" finite channel samples across "<<archives<<" archives\n";
        }
        else if(argc==6&&std::string(argv[1])=="--render-animation"){
            auto bytes=loadLocal(std::filesystem::u8path(argv[2])),animationBytes=loadLocal(std::filesystem::u8path(argv[3]));BinaryView view(bytes);
            auto animation=decodeBck(BinaryView(animationBytes));size_t consumed=0;float frame=std::stof(argv[4],&consumed);assetCheck(consumed==std::strlen(argv[4]),"Invalid frame argument");
            auto model=decodeModel(view);auto rig=animation.pose(decodeRig(view),frame);auto mesh=decodeMesh(view,model,rig);
            auto triangles=materialPoseTriangles(view,model,rig,mesh);auto hardware=renderMesh(triangles,false,720,720,&model.textures),reference=renderMesh(triangles,true,720,720,&model.textures);
            size_t differences=0;for(size_t i=0;i<hardware.pixels.size();++i){auto a=hardware.pixels[i],b=reference.pixels[i];if(std::abs(int(a.r)-b.r)>2||std::abs(int(a.g)-b.g)>2||std::abs(int(a.b)-b.b)>2)++differences;}
            assetCheck(differences<hardware.pixels.size()/100,"Animated hardware/WARP disagreement");saveNewResource(std::filesystem::u8path(argv[5]),textureBmp(hardware));
            std::cout<<"PASS: BCK joints="<<animation.joints.size()<<" duration="<<animation.duration<<" sampled frame="<<frame<<" triangles="<<triangles.size()/3<<" hardware/WARP mismatches="<<differences<<'\n';
        }
        else if(argc==4&&(std::string(argv[1])=="--render-mesh"||std::string(argv[1])=="--render-materials")){
            meshRenderTests();auto bytes=loadLocal(std::filesystem::u8path(argv[2]));BinaryView view(bytes);auto model=decodeModel(view);auto rig=decodeRig(view);auto mesh=decodeMesh(view,model,rig);
            bool materials=std::string(argv[1])=="--render-materials";auto triangles=materials?materialPoseTriangles(view,model,rig,mesh):defaultPoseTriangles(model,rig,mesh);auto textureData=materials?&model.textures:nullptr;
            auto reference=renderMesh(triangles,true,720,720,textureData),hardware=renderMesh(triangles,false,720,720,textureData);
            size_t differences=0,painted=0;for(size_t i=0;i<hardware.pixels.size();++i){auto a=hardware.pixels[i],b=reference.pixels[i];if(std::abs(int(a.r)-b.r)>2||std::abs(int(a.g)-b.g)>2||std::abs(int(a.b)-b.b)>2)++differences;if(a.r!=24||a.g!=28||a.b!=37)++painted;}
            assetCheck(painted>1000,"Hardware mesh render is empty");assetCheck(differences<hardware.pixels.size()/100,"Hardware/WARP mesh rendering disagreement");
            saveNewResource(std::filesystem::u8path(argv[3]),textureBmp(hardware));std::cout<<"PASS: rendered "<<triangles.size()/3<<" native skinned triangles, "<<painted<<" geometry pixels; hardware/WARP mismatches="<<differences<<"; depth order test passed\n";
        }
        else if(argc==3&&(std::string(argv[1])=="--preview-model"||std::string(argv[1])=="--preview-model-smoke"||std::string(argv[1])=="--preview-materials"||std::string(argv[1])=="--preview-materials-smoke")){
            auto bytes=loadLocal(std::filesystem::u8path(argv[2]));BinaryView view(bytes);auto model=decodeModel(view);auto rig=decodeRig(view);auto mesh=decodeMesh(view,model,rig);
            bool materials=std::string(argv[1]).find("materials")!=std::string::npos;auto triangles=materials?materialPoseTriangles(view,model,rig,mesh):defaultPoseTriangles(model,rig,mesh);TextureImage image;auto textures=materials?&model.textures:nullptr;
            try{image=renderMesh(triangles,false,720,720,textures);}catch(const std::exception&){image=renderMesh(triangles,true,720,720,textures);}
            bool smoke=std::string(argv[1]).find("smoke")!=std::string::npos;previewTexture(image,smoke);
            if(smoke)std::cout<<"PASS: native skinned model rendering and static preview-window lifecycle\n";
        }
        else if (argc == 3 && (std::string(argv[1]) == "--verify-graphics"||std::string(argv[1]) == "--verify-mesh"||std::string(argv[1])=="--verify-materials")) {
            bool verifyMaterials=std::string(argv[1])=="--verify-materials";bool verifyMesh=std::string(argv[1])=="--verify-mesh"||verifyMaterials;size_t meshPackets=0,meshTriangles=0;
            WiiAssets assets(std::filesystem::u8path(argv[2]));size_t archives=0,models=0,textures=0,vertices=0;uint64_t pixels=0;
            auto tally=[&](const TextureImage& t){++textures;pixels+=t.pixels.size();};
            std::cout<<"archive\tresource\ttype\tpositions\ttextures\n";
            for(const auto& file:assets.files) if(file.path.size()>=4&&file.path.substr(file.path.size()-4)==".arc") {
                Rarc archive(assets.readFile(file.path));++archives;
                for(const auto& resource:archive.files) {
                    std::string ext=std::filesystem::path(resource.path).extension().string();
                    if(ext!=".bmd"&&ext!=".bdl"&&ext!=".tpl"&&ext!=".bti") continue;
                    try {
                        auto bytes=archive.readFile(resource.path);BinaryView view(bytes);
                        if(ext==".bdl"||ext==".bmd") {
                            auto model=decodeModel(view);auto rig=decodeRig(view);++models;vertices+=model.positions.size();
                            if(verifyMesh){auto mesh=decodeMesh(view,model,rig);auto draw=skinDrawMatrices(rig);for(const auto& packet:mesh.packets)skinPacket(model,packet,draw);meshPackets+=mesh.packets.size();meshTriangles+=mesh.triangles;
                                if(verifyMaterials){auto sections=modelSections(view);VertexArrays arrays(sections.at("VTX1"));decodeMaterials(view,model);for(const auto& packet:mesh.packets)for(const auto& vertex:packet.vertices)arrays.read(vertex);}}
                            for(const auto& t:model.textures)tally(t);
                            std::cout<<file.path<<'\t'<<resource.path<<'\t'<<ext<<'\t'<<model.positions.size()<<'\t'<<model.textures.size()<<'\n';
                        } else {
                            auto ts=ext==".tpl"?decodeTpl(view):std::vector<TextureImage>{decodeBti(view)};
                            for(const auto& t:ts)tally(t);
                            std::cout<<file.path<<'\t'<<resource.path<<'\t'<<ext<<"\t0\t"<<ts.size()<<'\n';
                        }
                    } catch(const std::exception& e) {throw std::runtime_error(file.path+":"+resource.path+": "+e.what());}
                }
                if(archives%200==0)std::cerr<<"Checked graphics in "<<archives<<" archives\n";
            }
            std::cerr<<"PASS: "<<models<<" model containers, "<<vertices<<" positions, "<<textures<<" base textures, "<<pixels<<" decoded pixels across "<<archives<<" archives.\n";
            if(verifyMesh)std::cerr<<"PASS: "<<meshPackets<<" mesh packets, "<<meshTriangles<<" triangles with checked position and matrix references\n";
            if(verifyMaterials)std::cerr<<"PASS: indexed vertex appearance and material texture/shape associations across all models\n";
        }
        else if (argc == 3 && std::string(argv[1]) == "--list-assets") {
            WiiAssets assets(std::filesystem::u8path(argv[2]));
            std::cout << "path\tbytes\tpartition_offset\n";
            for (const auto& file : assets.files) std::cout << file.path << '\t' << file.size << '\t' << file.offset << '\n';
            std::cerr << "Indexed " << assets.files.size() << " files; " << assets.verifiedClusters() << " clusters passed H0 checks.\n";
        }
        else if (argc == 4 && std::string(argv[1]) == "--archive") {
            WiiAssets assets(std::filesystem::u8path(argv[2]));
            Rarc archive(assets.readFile(argv[3]));
            std::cout << "path\tbytes\n";
            for (const auto& file : archive.files) std::cout << file.path << '\t' << file.size << '\n';
            std::cerr << "Loaded " << archive.files.size() << " archive resources; " << assets.verifiedClusters() << " clusters passed H0 checks.\n";
        }
        else if (argc == 6 && std::string(argv[1]) == "--extract-resource") {
            WiiAssets assets(std::filesystem::u8path(argv[2]));
            Rarc archive(assets.readFile(argv[3]));
            auto resource=archive.readFile(argv[4]);
            saveNewResource(std::filesystem::u8path(argv[5]),resource);
            std::cout << "Saved " << resource.size() << " bytes from " << argv[3] << ':' << argv[4] << '\n';
        }
        else if (argc == 3 && std::string(argv[1]) == "--verify-archives") {
            WiiAssets assets(std::filesystem::u8path(argv[2]));
            size_t count=0, resources=0; uint64_t bytes=0;
            std::cout << "archive\tresources\tcompressed_bytes\n";
            for (const auto& file : assets.files) if (file.path.size() >= 4 && file.path.substr(file.path.size()-4)==".arc") {
                try {
                    Rarc archive(assets.readFile(file.path));
                    resources += archive.files.size(); bytes += file.size; ++count;
                    std::cout << file.path << '\t' << archive.files.size() << '\t' << file.size << '\n';
                    if (count % 100 == 0) std::cerr << "Verified " << count << " archives\n";
                } catch (const std::exception& e) { throw std::runtime_error(file.path + ": " + e.what()); }
            }
            std::cerr << "PASS: " << count << " archives, " << resources << " resources, " << bytes << " stored bytes; " << assets.verifiedClusters() << " clusters passed H0 checks.\n";
        }
        else { std::cout << "Galaxy native development probe (not a playable game)\n"
                               "  galaxy-probe --self-test\n  galaxy-probe --inspect <game.wbfs>\n"
                               "  galaxy-probe --list-assets <game.wbfs>\n  galaxy-probe --archive <game.wbfs> </path.arc>\n"
                               "  galaxy-probe --verify-archives <game.wbfs>\n"
                               "  galaxy-probe --decode-texture <texture.tpl-or-bti> <new-preview.bmp>\n"
                               "  galaxy-probe --preview-texture <texture.tpl-or-bti>\n"
                               "  galaxy-probe --preview-smoke <texture.tpl-or-bti>\n"
                               "  galaxy-probe --render-test <texture.tpl-or-bti> <new-readback.bmp>\n"
                               "  galaxy-probe --model <model.bdl-or-bmd>\n  galaxy-probe --verify-graphics <game.wbfs>\n"
                               "  galaxy-probe --mesh <model.bdl-or-bmd>\n  galaxy-probe --verify-mesh <game.wbfs>\n"
                               "  galaxy-probe --export-bind-obj <model.bdl-or-bmd> <new-output.obj>\n"
                               "  galaxy-probe --render-mesh <model.bdl-or-bmd> <new-preview.bmp>\n"
                               "  galaxy-probe --preview-model <model.bdl-or-bmd>\n"
                               "  galaxy-probe --render-materials <model.bdl-or-bmd> <new-preview.bmp>\n"
                               "  galaxy-probe --render-animation <model.bdl-or-bmd> <animation.bck> <frame> <new-preview.bmp>\n"
                               "  galaxy-probe --verify-animations <game.wbfs>\n"
                               "  galaxy-probe --animation-timeline <animation.bck> <tick-count>\n"
                               "  galaxy-probe --preview-animation <model.bdl> <animation.bck>\n"
                               "  galaxy-probe --stage-tables <game.wbfs> </StageData/archive.arc>\n"
                               "  galaxy-probe --stage-placements <game.wbfs> </StageData/archive.arc>\n"
                               "  galaxy-probe --preview-stateful-animation <game.wbfs> <stage.arc> <table> <row> <animation.arc> <clip.bck>\n"
                               "  galaxy-probe --preview-stateful-placement <game.wbfs> </StageData/archive.arc> </jmp/table> <row>\n"
                               "  galaxy-probe --render-stateful-placement <game.wbfs> </StageData/archive.arc> </jmp/table> <row> <new-preview.bmp>\n"
                               "  galaxy-probe --render-placement <game.wbfs> </StageData/archive.arc> </jmp/table> <row> <new-preview.bmp>\n"
                               "  galaxy-probe --render-zone <game.wbfs> </StageData/archive.arc> <new-preview.bmp>\n"
                               "  galaxy-probe --preview-zone <game.wbfs> </StageData/archive.arc>\n"
                               "  galaxy-probe --render-collision <mesh.kcl> <new-preview.bmp>\n"
                               "  galaxy-probe --verify-collision <mesh.kcl>\n"
                               "  galaxy-probe --verify-zone-collision <game.wbfs> </StageData/archive.arc>\n"
                               "  galaxy-probe --verify-zone-body <game.wbfs> </StageData/archive.arc>\n"
                               "  galaxy-probe --preview-zone-body <game.wbfs> </StageData/archive.arc>\n"
                               "  galaxy-probe --zone-point-gravity <game.wbfs> </StageData/archive.arc>\n"
                               "  galaxy-probe --zone-disk-gravity <game.wbfs> </StageData/archive.arc>\n"
                               "  galaxy-probe --zone-gravity <game.wbfs> </StageData/archive.arc>\n"
                               "  galaxy-probe --verify-gravity-body <game.wbfs> </StageData/archive.arc>\n"
                               "  galaxy-probe --preview-gravity-body <game.wbfs> </StageData/archive.arc>\n"
                               "  galaxy-probe --scenario-layers <game.wbfs> <galaxy-name> <scenario-number>\n"
                               "  galaxy-probe --scenario-placement-tree <game.wbfs> <galaxy-name> <scenario-number>\n"
                               "  galaxy-probe --render-scenario-zone <game.wbfs> <galaxy-name> <scenario-number> <zone-name> <new-preview.bmp>\n"
                               "  galaxy-probe --verify-scenario-collision <game.wbfs> <galaxy-name> <scenario-number> <zone-name>\n"
                               "  galaxy-probe --scenario-gravity <game.wbfs> <galaxy-name> <scenario-number> <zone-name>\n"
                               "  galaxy-probe --verify-scenario-body <game.wbfs> <galaxy-name> <scenario-number> <zone-name>\n"
                               "  galaxy-probe --preview-scenario-body <game.wbfs> <galaxy-name> <scenario-number> <zone-name>\n"
                               "  galaxy-probe --verify-native-jmap <game.wbfs> </StageData/archive.arc>\n"
                               "  galaxy-probe --verify-placed-actors <game.wbfs> </StageData/archive.arc>\n"
                               "  galaxy-probe --verify-transforming-scenario-actors <game.wbfs> <Galaxy> <ScenarioNo>\n"
                               "  galaxy-probe --verify-moving-scenario-actors <game.wbfs> <Galaxy> <ScenarioNo>\n"
                               "  galaxy-probe --verify-stateful-scenario-actors <game.wbfs> <Galaxy> <ScenarioNo>\n"
                               "  galaxy-probe --render-mario-track-blend <game.wbfs> <new.bmp>\n"
                               "  galaxy-probe --preview-mario-logical-animation <game.wbfs> <Shift-JIS-name-file>\n"
                               "  galaxy-probe --verify-mario-animation-groups <game.wbfs>\n"
                               "  galaxy-probe --verify-bck-settings <game.wbfs> <animation.arc> <control.banmt>\n"
                               "  galaxy-probe --verify-scenario-actors <game.wbfs> <Galaxy> <ScenarioNo>\n"
                               "  galaxy-probe --preview-scenario-mario <game.wbfs> <Galaxy> <ScenarioNo> <Zone> [MarioNo]\n"
                               "  galaxy-probe --render-scenario-mario <game.wbfs> <Galaxy> <ScenarioNo> <Zone> <new.bmp>\n"
                               "  galaxy-probe --verify-segment-gravity <game.wbfs>\n"
                               "  galaxy-probe --scenario-starts <game.wbfs> <Galaxy> <ScenarioNo>\n"
                               "  galaxy-probe --preview-materials <model.bdl-or-bmd>\n  galaxy-probe --verify-materials <game.wbfs>\n"
                               "  galaxy-probe --extract-resource <game.wbfs> </archive.arc> </resource> <new-output-file>\n"; return argc == 1 ? 0 : 2; }
        return 0;
    } catch (const std::exception& e) { std::cerr << "ERROR: " << e.what() << '\n'; return 1; }
}































































































































