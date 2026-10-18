#include "layout_text_geometry.hpp"
#include "miss_ui_resource.hpp"
#include "layout_composition.hpp"
#pragma once
#include "placed_actor.hpp"
#include "scenario_assembly.hpp"
#include "stage_switch_state.hpp"
#include "coin_rotation.hpp"
#include "coin_rewards.hpp"
#include "sensor_messages.hpp"
#include "sensor_world.hpp"
#include "scenario_gravity.hpp"
#include "coin_pose.hpp"
#include "galaxy_progress.hpp"
#include "stage_result.hpp"
#include "stage_memory.hpp"
#include "restart_cube.hpp"
#include "death_area.hpp"
#include "player_death_sequence.hpp"
#include "Game/Screen/MissLayout.hpp"
#include "Game/Screen/WipeKoopa.hpp"
#include "layout_pose.hpp"
// Owns active objinfo adapters and their original-reader table lifetimes.
class NativeScenarioActors {
    struct CoinVisualHost:LiveActor{ActorAppearSwitchListener appearance;std::function<void()> reward;HitSensor* sensor=nullptr;CoinVisualHost():appearance(this,true,true){}void appear()override{LiveActor::appear();if(sensor)sensor->validateBySystem();}void kill()override{if(sensor)sensor->invalidateBySystem();LiveActor::kill();}bool receiveOtherMsg(u32 message,HitSensor*,HitSensor*)override{if(message!=ACTMES_ITEM_GET||isDead()||!reward)return false;reward();kill();return true;}};
    std::map<std::string,std::unique_ptr<CoinVisualHost>> coinHosts;
    struct PickupPlayer:LiveActor{void attackSensor(HitSensor* own,HitSensor* other)override{MR::tryGetItem(own,other);}} pickupPlayer;
    NativeSensorWorld coinSensorWorld;HitSensor* pickupSensor=nullptr;
    NativeScene scene;MissLayout* missLayout=nullptr;WipeKoopa* downWipe=nullptr;NativeLayoutAssets downWipeResources;NativeMissUiResources missUiResources;NativeLayoutAnimation downWipeAnimation{};NativeCoinRotater* coinRotation=nullptr;ScenePlayingResult* playingResult=nullptr;NativePlayerStock playerStock;NativePlayerDeathSequence playerDeath;NativeCoinRewardEvents rewardEvents;std::shared_ptr<NativeGalaxyProgress> progress;int progressScenario=0;std::string progressGalaxy;NativeStageMemory stageMemory;GameDataTemporaryInGalaxy& stageResult=stageMemory.result;std::map<std::string,int> zoneIds;std::vector<NativeRestartCube*> restartCubes;std::vector<NativeDeathArea*> deathAreas;
    std::map<std::string,std::shared_ptr<NativeJmapTable>> tables;std::vector<std::string> actorZones;std::vector<std::unique_ptr<NativeStageSwitchWatcher>> watchers;bool updatingWatchers=false;std::vector<NativeStageSwitchWatcher*> scopedWatchers;
public:
    std::vector<NativePlacedActor*> actors;NativeStageSwitchState switches;
    NativeScenarioActors(WiiAssets& assets,const ScenarioAssembly& assembly,NativeCampaignProgress* campaign=nullptr){if(!assembly.galaxy.empty()&&assembly.scenarioNumber>=1&&assembly.scenarioNumber<=8&&assembly.powerStarCount<=8){progress=campaign?campaign->galaxy(assembly.galaxy,int(assembly.powerStarCount)):std::make_shared<NativeGalaxyProgress>(assembly.galaxy,int(assembly.powerStarCount));progressScenario=int(assembly.scenarioNumber);progressGalaxy=assembly.galaxy;progress->scenario(progressScenario).setFlagAlreadyVisited(true);}rewardEvents.deliverLife=[this](){playerStock.addPlayerLeft(1);};coinRotation=&scene.create<NativeCoinRotater>();playingResult=&scene.create<ScenePlayingResult>();missLayout=&scene.create<MissLayout>();missLayout->init(JMapInfoIter());playerDeath.bindMissLayout(*missLayout);downWipe=&scene.create<WipeKoopa>();downWipe->init(JMapInfoIter());playerDeath.bindDownWipe(*downWipe);{Rarc archive(assets.readFile("/LayoutData/WipeKoopa.arc"));downWipeResources=loadNativeLayoutAssets(archive,"/blyt/wipekoopa.brlyt");auto bytes=archive.readFile("/anim/out.brlan");downWipeAnimation=decodeLayoutAnimation(BinaryView(bytes));auto timing=downWipeAnimation.metadata;downWipe->registerNativeAnimation("out",timing.frames,timing.loop);}{missUiResources=loadNativeMissUi(assets);for(const auto& entry:missUiResources.animations){auto timing=entry.second.metadata;missLayout->registerNativeAnimation(entry.first,timing.frames,timing.loop);}}auto& pickupKeeper=coinSensorWorld.createKeeperForType(1,1);pickupSensor=&pickupKeeper.add("body",1,128,1.0f,pickupPlayer,{});pickupKeeper.validate();pickupSensor->invalidateBySystem();std::map<std::string,ZoneGravityField> coinGravity;
        if(!assembly.galaxy.empty()){auto zones=loadScenarioLayers(assets,assembly.galaxy,assembly.scenarioNumber);for(size_t i=0;i<zones.size();++i)zoneIds.emplace(zones[i].name,int(i));}
        std::map<std::string,std::unique_ptr<Rarc>> archives;
        for(const auto& record:assembly.placements){
            auto leaf=std::filesystem::path(record.placement.table).filename();bool restart=leaf=="areaobjinfo"&&record.placement.name=="RestartCube";bool death=leaf=="areaobjinfo"&&(record.placement.name=="DeathCube"||record.placement.name=="DeathSphere"||record.placement.name=="DeathCylinder");if(leaf!="objinfo"&&!restart&&!death)continue;
            auto key=record.zone+":"+record.placement.table;
            auto& table=tables[key];
            if(!table){auto& archive=archives[record.zone];if(!archive)archive=std::make_unique<Rarc>(assets.readFile("/StageData/"+record.zone+".arc"));auto bytes=archive->readFile(record.placement.table);table=std::make_shared<NativeJmapTable>(BinaryView(bytes));}
            if(death){
                for(float scale:record.placement.scale)assetCheck(scale>0,"Non-positive death area scale");int form=record.placement.name=="DeathCube"?1:record.placement.name=="DeathSphere"?2:3;const auto& ids=record.placement.switches;
                auto& area=scene.place<NativeDeathArea>(JMapInfoIter(&table->info,int(record.placement.row)),form,record.world,switches,record.zone,ids);deathAreas.push_back(&area);
                if(ids[2]>=0){area.invalidate();auto watcher=std::make_unique<NativeStageSwitchWatcher>(switches,record.zone,std::array<int32_t,3>{ids[0],ids[1],ids[2]},ids[3]);auto pointer=&area;watcher->listeners[2]=[pointer](bool enabled){if(enabled)pointer->validate();else pointer->invalidate();};watchers.push_back(std::move(watcher));}continue;
            }
            if(restart){
                auto zone=zoneIds.find(record.zone);assetCheck(zone!=zoneIds.end(),"Restart cube zone absent from scenario");for(float scale:record.placement.scale)assetCheck(scale>0,"Non-positive restart cube scale");
                NativePlacedInfoScope context(JMapInfoIter(&table->info,int(record.placement.row)),zone->second);auto& cube=scene.place<NativeRestartCube>(JMapInfoIter(&table->info,int(record.placement.row)),record.world);restartCubes.push_back(&cube);
                const auto& ids=record.placement.switches;for(auto id:ids)if(id>=0)switches.registerSwitch(record.zone,id);
                if(ids[2]>=0){cube.invalidate();auto watcher=std::make_unique<NativeStageSwitchWatcher>(switches,record.zone,std::array<int32_t,3>{ids[0],ids[1],ids[2]},ids[3]);auto pointer=&cube;watcher->listeners[2]=[pointer](bool enabled){if(enabled)pointer->validate();else pointer->invalidate();};watchers.push_back(std::move(watcher));}
                continue;
            }
            auto& actor=scene.place<NativePlacedActor>(JMapInfoIter(&table->info,int(record.placement.row)),table,record.parent);
            actor.placement.table=record.placement.table;
            assetCheck(actor.world==record.world&&actor.placement.name==record.placement.name&&actor.placement.arguments==record.placement.arguments&&actor.placement.switches==record.placement.switches,"Scenario actor world transform/name mismatch");for(auto id:actor.placement.switches)if(id>=0)switches.registerSwitch(record.zone,id);actors.push_back(&actor);actorZones.push_back(record.zone);if(record.placement.name=="Coin"||record.placement.name=="PurpleCoin"){auto host=std::make_unique<CoinVisualHost>();auto pointer=host.get();bool purple=record.placement.name=="PurpleCoin";host->reward=[this,purple](){NativeCoinRewardScope scope(rewardEvents);if(purple)playingResult->incPurpleCoin();else playingResult->incCoin(1);};auto& ids=record.placement.switches;if(ids[2]>=0)host->kill();auto& coinSensor=coinSensorWorld.create(0x4A,4,55.0f,*host);host->sensor=&coinSensor;auto gravityEntry=coinGravity.find(record.zone);if(gravityEntry==coinGravity.end())gravityEntry=coinGravity.emplace(record.zone,loadScenarioZoneGravity(assets,assembly,record.zone)).first;CollisionPoint gravity{},position{record.world[3],record.world[7],record.world[11]};if(auto selected=selectGravity(gravityEntry->second.sources,position))gravity=selected->direction;auto pose=nativeCoinPose(record.world,gravity,record.placement.arguments[4]>0);CollisionPoint up{pose[1],pose[5],pose[9]};float upLength=std::sqrt(collisionDot(up,up));assetCheck(upLength>0,"Coin sensor has zero up axis");coinSensor.mPosition={position[0]+70*up[0]/upLength,position[1]+70*up[1]/upLength,position[2]+70*up[2]/upLength};if(host->isDead())coinSensor.invalidateBySystem();coinHosts.emplace(scenarioActorKey(record),std::move(host));auto& watcher=watchActor(actors.size()-1);if(ids[2]>=0)watcher.listeners[2]=[pointer](bool enabled){pointer->appearance.refresh(enabled);};if(ids[1]>=0)watcher.listeners[1]=[pointer](bool enabled){if(enabled)pointer->kill();};}
        }
        assetCheck(scene.size()==actors.size()+restartCubes.size()+deathAreas.size()+4,"Scenario actor ownership mismatch");
    }
    bool beginFatalSequenceAt(const CollisionPoint& position){if(playerDeath.running()||!requestFatalAreaAt(position))return false;return playerDeath.start(playerStock);}
    void finishDiagnosticDeathRetry(){assetCheck(!playerDeath.running()&&playerDeath.services.endMiss==1,"Diagnostic retry before original miss completion");downWipe->forceOpen();downWipe->kill();missLayout->kill();}
    size_t composeDeathUi(TextureImage& frame)const{size_t painted=0;if(!missLayout->nativeDead&&!missLayout->nativeHidden)painted=compositeLayoutText(frame,missUiPose(),missUiResources.fonts);return painted+composeClosingWipe(frame);}
    size_t composeClosingWipe(TextureImage& frame)const{if(downWipe->nativeDead||downWipe->nativeHidden)return 0;return compositeLayoutWindows(frame,closingWipePose(),downWipeResources.textures);}
    NativeLayoutResource closingWipePose()const{return sampleLayoutPose(downWipeResources.layout,downWipeAnimation,downWipe->nativeAnimationFrame());}
    NativeLayoutResource missUiPose()const{if(missLayout->nativeAnimation.empty())return missUiResources.layout;auto animation=missUiResources.animations.find(missLayout->nativeAnimation);assetCheck(animation!=missUiResources.animations.end(),"Missing native miss animation resource");return sampleLayoutPose(missUiResources.layout,animation->second,missLayout->nativeAnimationFrame());}
    MissLayout& missLayoutActor(){return *missLayout;}
    NativePlayerDeathSequence& playerDeathSequence(){return playerDeath;}
    size_t fatalAreaCount()const{return deathAreas.size();}
    const NativeDeathArea& fatalArea(size_t index)const{assetCheck(index<deathAreas.size(),"Invalid fatal-area index");return *deathAreas[index];}
    bool evaluateFatalArea(size_t index,const CollisionPoint& point){assetCheck(index<deathAreas.size(),"Invalid fatal-area index");return deathAreas[index]->evaluate({point[0],point[1],point[2]});}
    bool requestFatalAreaAt(const CollisionPoint& point){for(auto area:deathAreas)if(area->evaluate({point[0],point[1],point[2]}))return true;return false;}
    const ScenarioPlacement* restartPlacement(const ScenarioAssembly& assembly)const{
        const auto& id=stageMemory.restartInfo();for(const auto& [zone,number]:zoneIds)if(number==id.mZoneID)return findScenarioStart(assembly,zone,id._0);return nullptr;
    }
    size_t restartCubeCount()const{return restartCubes.size();}
    const NativeRestartCube& restartCube(size_t index)const{assetCheck(index<restartCubes.size(),"Invalid restart cube index");return *restartCubes[index];}
    bool updatePlayerCheckpoint(const CollisionPoint& position,bool grounded){
        TVec3f point{position[0],position[1],position[2]};for(auto iter=restartCubes.rbegin();iter!=restartCubes.rend();++iter)if((*iter)->isInVolume(point)){(*iter)->updateRestart(stageMemory,grounded);return true;}return false;
    }
    NativeStageMemory& temporaryStageMemory(){return stageMemory;}
    uint32_t setupActorDoneFlag(size_t index,const char* name,uint32_t& value){
        assetCheck(index<actors.size(),"Invalid stage flag actor");auto zone=zoneIds.find(actorZones[index]);assetCheck(zone!=zoneIds.end(),"Actor zone absent from original zone list");
        auto& placement=actors[index]->placement;auto table=tables.find(actorZones[index]+":"+placement.table);assetCheck(table!=tables.end(),"Actor stage flag table unavailable");
        return stageMemory.setup(name,JMapInfoIter(&table->second->info,int(placement.row)),zone->second,value);
    }
    NativeStageSwitchWatcher& watchActor(size_t index){assetCheck(!updatingWatchers&&index<actors.size(),"Invalid actor switch watcher registration");auto& ids=actors[index]->placement.switches;watchers.push_back(std::make_unique<NativeStageSwitchWatcher>(switches,actorZones[index],std::array<int32_t,3>{ids[0],ids[1],ids[2]},ids[3]));return *watchers.back();}
    void unwatchActor(NativeStageSwitchWatcher& watcher){assetCheck(!updatingWatchers,"Watcher removal during scenario dispatch");assetCheck(std::find(scopedWatchers.begin(),scopedWatchers.end(),&watcher)==scopedWatchers.end(),"Removal of scoped actor watcher");auto found=std::find_if(watchers.begin(),watchers.end(),[&](const auto& value){return value.get()==&watcher;});assetCheck(found!=watchers.end(),"Watcher is not owned by this scenario");watchers.erase(found);}
    void tick(){assetCheck(!updatingWatchers,"Recursive scenario switch update");scene.tick();playerDeath.tick();updatingWatchers=true;try{for(auto& watcher:watchers)watcher->tick();updatingWatchers=false;}catch(...){updatingWatchers=false;throw;}}
    void withActorWatcher(size_t index,const std::function<void(NativeStageSwitchWatcher&)>& callback){auto& watcher=watchActor(index);try{scopedWatchers.push_back(&watcher);}catch(...){unwatchActor(watcher);throw;}try{callback(watcher);}catch(...){scopedWatchers.pop_back();unwatchActor(watcher);throw;}scopedWatchers.pop_back();unwatchActor(watcher);}
    void verifySwitchDispatch(){
        for(size_t index=0;index<actors.size();++index){auto& ids=actors[index]->placement.switches;
            for(size_t channel=0;channel<3;++channel){if(ids[channel]<0)continue;auto id=ids[channel];bool previous=switches.get(actorZones[index],id);
                try{
                    withActorWatcher(index,[&](NativeStageSwitchWatcher& scoped){bool blocked=false;try{unwatchActor(scoped);}catch(const std::exception&){blocked=true;}assetCheck(blocked,"Scoped watcher removal accepted");withActorWatcher(index,[&](NativeStageSwitchWatcher& nested){bool outerBlocked=false;try{unwatchActor(scoped);}catch(const std::exception&){outerBlocked=true;}assetCheck(outerBlocked&&&nested!=&scoped,"Nested scope lost outer watcher protection");});});switches.set(actorZones[index],id,false);bool rejected=false;
                    try{withActorWatcher(index,[&](NativeStageSwitchWatcher& watcher){watcher.listeners[channel]=[](bool){throw std::runtime_error("expected scoped watcher exception");};switches.set(actorZones[index],id,true);tick();});}catch(const std::runtime_error&){rejected=true;}
                    assetCheck(rejected,"Scoped watcher exception did not propagate");tick();
                    switches.set(actorZones[index],id,false);std::vector<bool> events;
                    withActorWatcher(index,[&](NativeStageSwitchWatcher& watcher){watcher.listeners[channel]=[&](bool enabled){events.push_back(enabled);};switches.set(actorZones[index],id,true);tick();tick();switches.set(actorZones[index],id,false);tick();assetCheck(events==std::vector<bool>({true,false}),"Authored actor switch callback delivery mismatch");});
                    switches.set(actorZones[index],id,true);tick();assetCheck(events==std::vector<bool>({true,false}),"Detached authored watcher still dispatched");int replacementEvents=0;
                    withActorWatcher(index,[&](NativeStageSwitchWatcher& watcher){watcher.listeners[channel]=[&](bool){++replacementEvents;};tick();assetCheck(replacementEvents==1,"Reattached authored watcher did not observe current switch");});
                }catch(...){switches.set(actorZones[index],id,previous);throw;}
                switches.set(actorZones[index],id,previous);return;
            }
        }
        throw std::runtime_error("No authored actor switch available for dispatch verification");
    }
    static void coinCollectionTests(){NativeScene scene;auto& result=scene.create<ScenePlayingResult>();NativeCoinRewardEvents events;CoinVisualHost host;host.reward=[&](){NativeCoinRewardScope scope(events);result.incCoin(1);};assetCheck(!host.receiveOtherMsg(ACTMES_PUSH,nullptr,nullptr)&&result.getCoinNum()==0,"Unrelated message collected coin");assetCheck(host.receiveOtherMsg(ACTMES_ITEM_GET,nullptr,nullptr)&&host.isDead()&&result.getCoinNum()==1,"Coin item-get did not reward/hide");assetCheck(!host.receiveOtherMsg(ACTMES_ITEM_GET,nullptr,nullptr)&&result.getCoinNum()==1,"Dead coin awarded twice");host.appear();host.reward=[&](){NativeCoinRewardScope scope(events);result.incPurpleCoin();};assetCheck(host.receiveOtherMsg(ACTMES_ITEM_GET,nullptr,nullptr)&&result.mPurpleCoinNum==1&&result.getCoinNum()==1,"Purple coin reward routed to normal counter");}
    static void coinVisibilityTests(){NativeStageSwitchState state;CoinVisualHost host;host.kill();NativeStageSwitchWatcher watcher(state,"test",{-1,1,2});watcher.listeners[1]=[&](bool enabled){if(enabled)host.kill();};watcher.listeners[2]=[&](bool enabled){host.appearance.refresh(enabled);};state.set("test",2,true);watcher.tick();assetCheck(!host.isDead(),"Coin appearance switch did not appear");state.set("test",1,true);watcher.tick();assetCheck(host.isDead(),"Coin B switch did not hide");state.set("test",1,false);watcher.tick();assetCheck(host.isDead(),"Coin B switch off revived coin");state.set("test",2,false);watcher.tick();state.set("test",2,true);watcher.tick();assetCheck(!host.isDead(),"Coin appearance event did not restore coin");}
    void collectCoinsAt(const CollisionPoint& position,float radius){assetCheck(std::isfinite(radius)&&radius>0,"Invalid pickup radius");pickupPlayer.mPosition={position[0],position[1],position[2]};pickupSensor->mRadius=radius;pickupSensor->validateBySystem();coinSensorWorld.tick();}
    CollisionPoint coinSensorPosition(const std::string& key)const{auto found=coinHosts.find(key);assetCheck(found!=coinHosts.end(),"Unknown coin sensor host");auto& p=found->second->sensor->mPosition;return {p.x,p.y,p.z};}
    bool tryCollectCoin(const std::string& key){auto found=coinHosts.find(key);assetCheck(found!=coinHosts.end(),"Unknown coin collection host");return MR::tryGetItem(nullptr,found->second->sensor);}
    bool scenarioVisited(){assetCheck(bool(progress),"Scenario progress unavailable");return progress->scenario(progressScenario).isAlreadyVisited();}
    bool scenarioStarOwned(){assetCheck(bool(progress),"Scenario progress unavailable");return progress->scenario(progressScenario).hasPowerStar();}
    void recordPowerStarCollected(int starId=-1){assetCheck(bool(progress),"Scenario progress unavailable");if(starId==-1)starId=progressScenario;auto accessor=progress->scenario(starId);stageResult.receiveStageResultParam(progressGalaxy.c_str(),starId,0,coinCount());accessor.setPowerStarFlag(true);accessor.updateMaxCoinNum(stageResult.mCoinNum);}
    const GameDataTemporaryInGalaxy& collectedStageResult()const{return stageResult;}
    int playerLives()const{return playerStock.getPlayerLeft();}
    int coinCount()const{return playingResult->getCoinNum();}
    int purpleCoinCount()const{return playingResult->mPurpleCoinNum;}
    bool coinVisible(const std::string& key)const{auto found=coinHosts.find(key);assetCheck(found!=coinHosts.end(),"Unknown coin visual host");return !found->second->isDead();}
    const NativeCoinRotater& coinRotater()const{return *coinRotation;}
    void renderPhases(){scene.renderPhases();}
};
































