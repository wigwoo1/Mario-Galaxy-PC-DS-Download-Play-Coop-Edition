#include <Native/CollisionCamera.hpp>
#include <Game/Camera/GameCameraCreator.hpp>
#include <algorithm>
#include <array>
#include <cassert>
#include <cstdio>
#include <cstring>
#include <stdexcept>
#include <string>
#include <vector>

struct Group { std::string host; int zone; unsigned code; };
class CameraParamChunkID_Tmp {
public:
 Group group;
 void createGroupID(int zone,const char* host,unsigned code,int){group={host,zone,code};}
};
class CameraParamChunkHolder {
public:
 std::vector<Group> groups;
 void createChunk(const CameraParamChunkID_Tmp& id,void*){groups.push_back(id.group);}
};
// ACTUAL_CREATOR_METHODS
struct CameraDirector {
 GameCameraCreator* mCameraCreator;
 int starts=0,registrations=0,finishes=0;
 void initCameraCodeCollection(const char* name,int zone){++starts;mCameraCreator->initCameraCodeCollection(name,zone);}
 void registerCameraCode(unsigned code){++registrations;mCameraCreator->registerCameraCode(code);}
 void termCameraCodeCollection(){++finishes;mCameraCreator->termCameraCodeCollection();}
};
struct PreviewScene {
 int zoneCount=2,cameraCollectionZone=-1;
 std::string cameraCollectionHost;
 std::array<bool,256> cameraCollectionCodes{};
 std::vector<Group> pendingCameraGroups;
};
static PreviewScene* active=nullptr;
static CameraDirector* gameDirector=nullptr;
static PreviewScene& current(){if(!active)throw std::logic_error("No active preview");return *active;}
namespace MR {CameraDirector* getCameraDirector(){return gameDirector;}}
// ACTUAL_DISPATCH_METHODS

int main(){
 CameraParamChunkHolder chunks;GameCameraCreator creator(&chunks);
 CameraDirector director{&creator};gameDirector=&director;
 {Native::CollisionCameraCollection game("terrain",1);MR::registerCameraCode(7);MR::registerCameraCode(7);MR::registerCameraCode(254);MR::registerCameraCode(255);game.commit();}
 assert(director.starts==1&&director.finishes==1&&director.registrations==4);
 assert(chunks.groups.size()==2&&chunks.groups[0].host=="terrain"&&chunks.groups[0].zone==1&&chunks.groups[0].code==7&&chunks.groups[1].code==254);
 {Native::CollisionCameraCollection failed("failed",0);MR::registerCameraCode(8);}
 assert(creator.mName==nullptr&&creator._8==-1&&chunks.groups.size()==2);
 assert(std::all_of(std::begin(creator.mCodes),std::end(creator.mCodes),[](u8 code){return code==0;}));
 {Native::CollisionCameraCollection retry("retry",0);MR::registerCameraCode(9);retry.commit();}
 assert(chunks.groups.size()==3&&chunks.groups.back().code==9&&chunks.groups.back().host=="retry");
 PreviewScene preview;active=&preview;
 {Native::CollisionCameraCollection local("preview",1);MR::registerCameraCode(10);MR::registerCameraCode(10);MR::registerCameraCode(255);local.commit();}
 assert(preview.pendingCameraGroups.size()==1&&preview.pendingCameraGroups[0].code==10);
 {Native::CollisionCameraCollection failed("failed preview",0);MR::registerCameraCode(11);}
 assert(preview.cameraCollectionZone==-1&&preview.cameraCollectionHost.empty()&&preview.pendingCameraGroups.size()==1);
 assert(director.starts==3&&director.finishes==2&&chunks.groups.size()==3);
 active=nullptr;
 std::puts("PASS actual camera collection dispatch: game director forwarding, original group creation/deduplication, sentinel rejection, abort/retry and isolated preview collection");
}
