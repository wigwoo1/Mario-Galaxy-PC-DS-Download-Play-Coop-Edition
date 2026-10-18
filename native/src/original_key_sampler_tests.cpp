#include "generated/OriginalKeySampler.hpp"
#include "animation.hpp"
#include "native_bck_tables.hpp"
#include <Native/BckAnimation.hpp>
#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <memory>
#include <vector>
namespace {
void putBE16(std::vector<u8>& bytes,size_t offset,u16 value){bytes.at(offset)=u8(value>>8);bytes.at(offset+1)=u8(value);}
void putBE32(std::vector<u8>& bytes,size_t offset,u32 value){bytes.at(offset)=u8(value>>24);bytes.at(offset+1)=u8(value>>16);bytes.at(offset+2)=u8(value>>8);bytes.at(offset+3)=u8(value);}
void putBEFloat(std::vector<u8>& bytes,size_t offset,float value){u32 bits;std::memcpy(&bits,&value,sizeof(bits));putBE32(bytes,offset,bits);}
#ifndef NATIVE_BCK_TEST_NO_BCA
void verifyOwnedBca(){
    constexpr size_t fileSize=124,block=32;
    std::vector<u8> bytes(fileSize);
    std::memcpy(bytes.data(),"J3D1bca1",8);putBE32(bytes,8,fileSize);putBE32(bytes,12,1);
    std::memcpy(bytes.data()+block,"ANF1",4);putBE32(bytes,block+4,92);
    bytes[block+8]=2;putBE16(bytes,block+10,10);putBE16(bytes,block+12,1);
    putBE32(bytes,block+20,36);putBE32(bytes,block+24,72);putBE32(bytes,block+28,80);putBE32(bytes,block+32,84);
    for(size_t axis=0;axis<3;++axis){size_t entry=block+36+axis*12;
        putBE16(bytes,entry,2);putBE16(bytes,entry+2,0);putBE16(bytes,entry+4,2);putBE16(bytes,entry+6,0);putBE16(bytes,entry+8,2);putBE16(bytes,entry+10,0);
    }
    putBEFloat(bytes,block+72,1.0f);putBEFloat(bytes,block+76,2.0f);
    putBE16(bytes,block+80,0);putBE16(bytes,block+82,100);
    putBEFloat(bytes,block+84,10.0f);putBEFloat(bytes,block+88,20.0f);
    std::unique_ptr<J3DAnmBase> owned(Native::createBcaAnimation(bytes.data(),bytes.size()));
    assert(owned && owned->getKind()==9);
    auto* animation=static_cast<J3DAnmTransformFull*>(owned.get());animation->mFrame=1.0f;
    J3DTransformInfo pose;animation->getTransform(0,&pose);
    assert(pose.mScale.x==2.0f&&pose.mScale.y==2.0f&&pose.mScale.z==2.0f);
    assert(pose.mRotation.x==100&&pose.mRotation.y==100&&pose.mRotation.z==100);
    assert(pose.mTranslate.x==20.0f&&pose.mTranslate.y==20.0f&&pose.mTranslate.z==20.0f);
    std::fill(bytes.begin(),bytes.end(),0xdd);animation->getTransform(0,&pose);assert(pose.mScale.x==2.0f&&pose.mTranslate.z==20.0f);
    auto malformed=std::vector<u8>(fileSize);std::memcpy(malformed.data(),"J3D1bca1",8);putBE32(malformed,8,fileSize);putBE32(malformed,12,1);
    std::memcpy(malformed.data()+block,"ANF1",4);putBE32(malformed,block+4,92);putBE16(malformed,block+10,10);putBE16(malformed,block+12,1);putBE32(malformed,block+20,0xfffffff0u);
    bool rejected=false;try{std::unique_ptr<J3DAnmBase> invalid(Native::createBcaAnimation(malformed.data(),malformed.size()));}catch(const std::runtime_error&){rejected=true;}assert(rejected);
    std::puts("PASS owned big-endian BCA transform tables, playback sample, source lifetime and malformed-offset rejection");
}
#endif
}
int main(int argc,char** argv){
#ifndef NATIVE_BCK_TEST_NO_BCA
    verifyOwnedBca();
#endif
    bool nullRejected=false;
    try{std::unique_ptr<J3DAnmBase> invalid(Native::createBckAnimation(nullptr,32));}
    catch(const std::runtime_error&){nullRejected=true;}
    assert(nullRejected);
    for(unsigned mode=0;mode<5;++mode){
        J3DFrameCtrl controller(10);controller.mAttribute=mode;
        for(unsigned tick=0;tick<25;++tick)controller.update();
        if(mode==0){assert(controller.mRate==0 && std::fabs(controller.mFrame-9.999f)<0.0001f);}
        if(mode==1){assert(controller.mRate==0 && controller.mFrame==0);}
        if(mode==2){assert(controller.mRate==1 && controller.mFrame==5);}
        if(mode==3){assert(controller.mRate==0 && controller.mFrame==1);}
        if(mode==4){assert(controller.mRate==1 && controller.mFrame==7);}
    }
    J3DAnmKeyTableBase shared{2,0,0},separate{2,0,1};
    float floats[]{0,2,3,10,32,3};s16 rotations[]{0,-100,20,10,100,20};
    float split[]{0,2,0,3,10,32,3,0};s16 signedSplit[]{0,-100,0,20,10,100,20,0};
    for(unsigned tick=0;tick<=100;++tick){float time=tick/10.0f;
        assert(std::fabs(J3DGetKeyFrameInterpolation(time,&shared,floats)-(2+3*time))<0.0001f);
        assert(std::fabs(J3DGetKeyFrameInterpolation(time,&separate,split)-(2+3*time))<0.0001f);
        assert(std::fabs(J3DGetKeyFrameInterpolation(time,&shared,rotations)-(-100+20*time))<0.0001f);
        assert(std::fabs(J3DGetKeyFrameInterpolation(time,&separate,signedSplit)-(-100+20*time))<0.0001f);
    }
    assert(J3DGetKeyFrameInterpolation(-1,&shared,rotations)==-100);
    assert(J3DGetKeyFrameInterpolation(11,&shared,rotations)==100);
    std::puts("PASS original float/s16 key sampler: shared/separate tangents, 404 samples and endpoint clamps");
    if(argc==2){
        WiiAssets disc(std::filesystem::u8path(argv[1]));Rarc archive(disc.readFile("/ObjectData/MarioAnime.arc"));
        size_t animations=0,samples=0;
        for(const auto& file:archive.files){
            auto bytes=archive.readFile(file.path);
            if(bytes.size()<8 || std::memcmp(bytes.data(),"J3D1bck1",8))continue;
            BinaryView view(bytes);auto decoded=decodeBck(view);auto block=view.sub(32,view.u32(36));
            NativeBckTables owned(view);
            J3DAnmTransformKey original;owned.bind(original);
            std::unique_ptr<J3DAnmBase> loaded;
            { // Poison and release the exact allocation passed into the loader.
                auto temporary=bytes;
                loaded.reset(Native::createBckAnimation(temporary.data(),temporary.size()));
                std::fill(temporary.begin(),temporary.end(),0xdd);
            }
            auto rejectBck=[&](std::vector<uint8_t> corrupt){
                bool rejected=false;
                try{std::unique_ptr<J3DAnmBase> invalid(Native::createBckAnimation(corrupt.data(),corrupt.size()));}
                catch(const std::runtime_error&){rejected=true;}
                assert(rejected);
            };
            auto corrupt=bytes;corrupt.resize(31);rejectBck(corrupt);
            corrupt=bytes;corrupt[41]=16;rejectBck(corrupt);
            corrupt=bytes;std::memset(corrupt.data()+52,0xff,4);rejectBck(corrupt);
            assert(loaded->getKind()==8);
            auto* loadedTransform=static_cast<J3DAnmTransformKey*>(loaded.get());
            J3DFrameCtrl controller(owned.duration);controller.mAttribute=owned.loop;
            for(unsigned tick=0;tick<120;++tick){
                original.mFrame=controller.mFrame;
                for(size_t joint=0;joint<decoded.joints.size();++joint){
                    J3DTransformInfo transform;original.getTransform(u16(joint),&transform);
                    for(unsigned axis=0;axis<3;++axis){
                        auto& tracks=decoded.joints[joint];
                        auto reference=tracks[axis*3].sample(controller.mFrame,1);
                        assert(std::fabs((&transform.mScale.x)[axis]-reference)<=0.0001f*(1+std::fabs(reference)));
                        reference=tracks[axis*3+2].sample(controller.mFrame,0);
                        assert(std::fabs((&transform.mTranslate.x)[axis]-reference)<=0.0001f*(1+std::fabs(reference)));
                        reference=tracks[axis*3+1].sample(controller.mFrame,0);
                        assert((&transform.mRotation.x)[axis]==s16(u16(u32(s32(reference))<<owned.rotationShift)));
                    }
                }
                controller.update();
            }
            assert(owned.axes.size()==decoded.joints.size()*3);
            size_t table=block.u32(20);
            for(size_t joint=0;joint<decoded.joints.size();++joint)for(unsigned channel=0;channel<9;++channel){
                size_t entry=table+joint*54+channel*6,kind=channel%3;
                u16 count=block.u16(entry),offset=block.u16(entry+2),type=block.u16(entry+4);
                size_t length=count>1?count*(type?4:3):count;
                size_t base=block.u32(24+kind*4);
                std::vector<float> floats;std::vector<s16> rotations;
                for(size_t value=0;value<length;++value){
                    if(kind==1)rotations.push_back(s16(block.u16(base+(offset+value)*2)));
                    else floats.push_back(block.f32(base+(offset+value)*4));
                }
                J3DAnmKeyTableBase info{count,0,type};
                auto axis=joint*3+channel/3;
                auto nativeInfo=kind==0?owned.axes[axis].mScaleInfo:kind==1?owned.axes[axis].mRotationInfo:owned.axes[axis].mTranslateInfo;
                assert(nativeInfo.mMaxFrame==count && nativeInfo.mOffset==offset && nativeInfo.mType==type);
                for(size_t value=0;value<length;++value){
                    if(kind==1)assert(owned.rotations[offset+value]==rotations[value]);
                    else assert((kind==0?owned.scales:owned.translations)[offset+value]==floats[value]);
                }
                for(unsigned tick=0;tick<=16;++tick){float frame=decoded.duration*tick/16.0f;
                    float actual=count==0?(kind==0?1.0f:0.0f):
                      kind==1?(count==1?float(rotations[0]):J3DGetKeyFrameInterpolation(frame,&info,rotations.data())):
                      (count==1?floats[0]:J3DGetKeyFrameInterpolation(frame,&info,floats.data()));
                    float reference=decoded.joints[joint][channel].sample(frame,kind==0?1:0);
                    original.mFrame=frame;J3DTransformInfo transform;
                    original.getTransform(u16(joint),&transform);
                    loadedTransform->mFrame=frame;J3DTransformInfo loadedPose;
                    loadedTransform->getTransform(u16(joint),&loadedPose);
                    for(unsigned axis=0;axis<3;++axis){assert((&transform.mScale.x)[axis]==(&loadedPose.mScale.x)[axis]);assert((&transform.mRotation.x)[axis]==(&loadedPose.mRotation.x)[axis]);assert((&transform.mTranslate.x)[axis]==(&loadedPose.mTranslate.x)[axis]);}
                    unsigned component=channel/3;
                    if(kind==1){auto expected=s16(u16(u32(s32(reference))<<decoded.rotationShift));
                        const s16* angles=&transform.mRotation.x;assert(angles[component]==expected);
                    }else {const float* values=kind==0?&transform.mScale.x:&transform.mTranslate.x;
                        assert(std::fabs(values[component]-reference)<=0.0001f*(1+std::fabs(reference)));
                    }
                    if(!std::isfinite(actual) || std::fabs(actual-reference)>0.0001f*(1+std::fabs(reference))){
                        std::printf("FAIL %s joint %zu channel %u frame %g: original %g reference %g\n",file.path.c_str(),joint,channel,frame,actual,reference);
                        return 1;
                    }
                    ++samples;
                }
            }
            ++animations;
        }
        assert(animations>0);
        std::puts("PASS owned BCK lifetime after source poisoning/release, malformed input rejection and virtual destruction");
        std::puts("PASS original frame controller: five modes and 120 playback updates per retail animation");
        std::printf("PASS %zu retail Mario BCK animations: %zu original sampler/reference comparisons\n",animations,samples);
    }
}




