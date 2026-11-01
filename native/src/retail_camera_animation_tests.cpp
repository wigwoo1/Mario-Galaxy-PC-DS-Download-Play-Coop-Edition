#include "assets.hpp"
#include <Native/OwnedCameraAnimation.hpp>
#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstddef>

// Match the old public object layout without changing CameraParamChunk's ABI.
struct LegacyCameraAnimationLayout: Camera {
    s32 a,b;u32 frames;bool keyed;f32 speed,current;
    BaseCamAnmDataAccessor* accessor;CamAnmDataAccessor* frame;KeyCamAnmDataAccessor* key;
    u32 values;s32 metadata;u8* resource;bool paused;
};
static_assert(sizeof(CameraAnim)==sizeof(LegacyCameraAnimationLayout));
static_assert(alignof(CameraAnim)==alignof(LegacyCameraAnimationLayout));
static_assert(offsetof(CameraAnim,mNrFrames)==offsetof(LegacyCameraAnimationLayout,frames));
static_assert(offsetof(CameraAnim,mFileDataAccessor)==offsetof(LegacyCameraAnimationLayout,accessor));
static_assert(offsetof(CameraAnim,mNativeAnimation)==offsetof(LegacyCameraAnimationLayout,resource));
static_assert(offsetof(CameraAnim,mIsPaused)==offsetof(LegacyCameraAnimationLayout,paused));

static std::array<float,8> sample(BaseCamAnmDataAccessor& sampler,float frame){
    TVec3f pos,watch;sampler.getPos(&pos,frame);sampler.getWatchPos(&watch,frame);
    return {pos.x,pos.y,pos.z,watch.x,watch.y,watch.z,sampler.getTwist(frame),sampler.getFovy(frame)};
}
int main(int argc,char**argv){try{
    std::setvbuf(stdout,nullptr,_IONBF,0);
    assetCheck(argc==2||argc==3,"Expected WBFS and optional --old");
    const bool old=argc==3;WiiAssets disc(std::filesystem::u8path(argv[1]));
    unsigned count=0,keyed=0,frames=0;
    for(const auto& f:disc.files){
        if(f.path.find("PeachCastleGardenGalaxy")==std::string::npos&&
           f.path.find("DemoLetter.arc")==std::string::npos&&
           f.path.find("DemoPeachCastleGate.arc")==std::string::npos&&
           f.path.find("AstroDomeDemoCamera.arc")==std::string::npos)continue;
        if(f.path.size()<4||f.path.substr(f.path.size()-4)!=".arc")continue;
        std::printf("ARCHIVE %s\n",f.path.c_str());
        Rarc archive(disc.readFile(f.path));
        for(const auto& member:archive.files){
            auto bytes=archive.readFile(member.path);
            if(bytes.size()<32||std::memcmp(bytes.data(),"ANDO",4))continue;
            if(old){
                if(std::memcmp(bytes.data()+4,"CKAN",4))continue;
                // Exact old raw-info/value binding; invoke original getPos in a
                // separate expected-failure process, not a fabricated null call.
                auto* header=reinterpret_cast<const CanmFileHeader*>(bytes.data());
                KeyCamAnmDataAccessor raw;
                raw.set(bytes.data()+32,bytes.data()+32+header->mValueOffset+4);
                std::printf("OLD %s:%s bytes=%zu raw-offset=%u authored-offset=%u\n",f.path.c_str(),member.path.c_str(),bytes.size(),header->mValueOffset,be32(bytes.data()+28));
                TVec3f pos;raw.getPos(&pos,0);
                std::fprintf(stderr,"Old raw camera reader unexpectedly survived (%f)\n",double(pos.x));return 3;
            }
            Native::CameraAnimationData decoded;
            assetCheck(decoded.decode(bytes.data(),bytes.size()),"Retail camera decode rejected");
            Native::OwnedCameraAnimation owned;assetCheck(owned.load(bytes.data(),bytes.size()),"Retail owned camera rejected");
            CamAnmDataAccessor linear;KeyCamAnmDataAccessor key;
            auto* sampler=owned.bind(linear,key);assert(sampler&&owned.frames()==be32(bytes.data()+24));
            for(unsigned t=0;t<owned.frames();++t){
                for(float fraction:{0.f,.5f}){
                    const auto result=sample(*sampler,float(t)+fraction);
                    for(float value:result)assert(std::isfinite(value));
                }
                ++frames;
            }
            if(decoded.keyed){
                ++keyed;
                for(unsigned ch=0;ch<8;++ch){
                    const auto c=decoded.components[ch];const unsigned stride=c.count>1?(c.type?4:3):1;
                    const float first=decoded.values[c.offset+(c.count>1?1:0)];
                    const float last=decoded.values[c.offset+(c.count-1)*stride+(c.count>1?1:0)];
                    const float before=c.count>1?decoded.values[c.offset]-1:0;
                    const float after=c.count>1?decoded.values[c.offset+(c.count-1)*stride]+1:0;
                    assert(sample(*sampler,before)[ch]==first&&sample(*sampler,after)[ch]==last);
                }
            }
            const auto retained=sample(*sampler,0);
            std::fill(bytes.begin(),bytes.end(),0);bytes.clear();bytes.shrink_to_fit();
            assert(sample(*sampler,0)==retained);
            assert(!owned.load(nullptr,0)&&sample(*sampler,0)==retained);
            std::printf("PASS %s:%s frames=%u keyed=%u values=%u\n",f.path.c_str(),member.path.c_str(),owned.frames(),unsigned(owned.keyed()),owned.valueCount());
            ++count;
        }
    }
    assetCheck(!old,"No original keyed camera was found for failure reproduction");
    assetCheck(count&&keyed,"Expected retail cameras including keyed opening data");
    std::printf("PASS %u retail cameras (%u keyed), %u full-frame plus half-frame samples; owned data survives input release; compatible camera object layout\n",count,keyed,frames);
}catch(const std::exception& e){std::fprintf(stderr,"%s\n",e.what());return 1;}}
