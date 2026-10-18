#define main originalHeapTestEntry
#include "unit_heap_live_tests.cpp"
#undef main
#include "native_owned_bas.hpp"
#include "native_disc_archive_loader.hpp"
#include "graphics.hpp"
#include <JSystem/JAudio2/JASSoundParams.hpp>
#include <array>
#include <cmath>
#include <tuple>

// ORIGINAL_SOUND_HANDLE_METHODS

static void putWord(std::vector<u8>& bytes,std::size_t at,u32 value){for(unsigned i=0;i<4;++i)bytes.at(at+i)=u8(value>>(24-i*8));}
static void putFloat(std::vector<u8>& bytes,std::size_t at,float value){u32 bits;std::memcpy(&bits,&value,4);putWord(bytes,at,bits);}
static std::vector<u8> syntheticBas(){
    std::vector<u8> bytes(8+3*32);bytes[1]=3;
    for(unsigned i=0;i<3;++i){auto at=8+i*32;putWord(bytes,at,0x12345678+i);putFloat(bytes,at+4,i?2.f:1.f);putFloat(bytes,at+8,i==1?6.f:(i?2.f:1.f));putFloat(bytes,at+12,.5f+i*.5f);bytes[at+20]=64;bytes[at+23]=64;}
    putWord(bytes,8+32+16,0x48);bytes[8+32+22]=1;
    putWord(bytes,8+64+16,0x81);bytes[8+64+22]=1;bytes[8+64+25]=3;
    bytes[8+21]=u8(-4);bytes[8+24]=u8(-2);return bytes;
}
static void checkMalformed(){
    auto valid=syntheticBas();
    auto reject=[&](const void* bytes,std::size_t size){bool caught=false;try{Native::BasAnimation invalid(bytes,size);}catch(const std::exception&){caught=true;}assert(caught);};
    reject(nullptr,8);reject(valid.data(),7);reject(valid.data(),valid.size()-1);
    auto bytes=valid;putWord(bytes,4,1);reject(bytes.data(),bytes.size());
    bytes=valid;putWord(bytes,12,0x7fc00000);reject(bytes.data(),bytes.size());
    bytes=valid;putWord(bytes,24,0x80);reject(bytes.data(),bytes.size());
    bytes=valid;bytes[0]=bytes[1]=0xff;reject(bytes.data(),bytes.size());
    std::array<u8,8> empty{};Native::BasAnimation none(empty.data(),empty.size());assert(!none.animation()->getNumSounds());
    Native::BasAnimation decoded(valid.data(),valid.size());auto* animation=decoded.animation();
    JAISoundHandles handles(nullptr,0);JAUSoundAnimator animator(&handles);
    animator.startAnimation(animation,false,2.f,2.f);assert(animator.mLoopStartSoundIndex==1&&animator.mLoopEndSoundIndex==3&&animator.mLoopSoundIndex==0);
    animator.startAnimation(animation,true,1.f,6.f);assert(animator.mLoopSoundIndex==2&&animator.mIsReversed);
    const auto* first=animation->getSound(0);assert(first->mPitchDelta==-4&&first->mVolumeDelta==-2&&u32(first->mSoundID)==0x12345678);
    JASSoundParams parameters;animator.modifySoundParams(&parameters,first,2.f);
    assert(std::abs(parameters.mPitch-.375f)<1e-6f&&std::abs(parameters.mVolume-60.f/127.f)<1e-6f&&std::abs(parameters.mPan-64.f/127.f)<1e-6f);
    TVec3f position(0,0,0);const auto* repeating=animation->getSound(2);
    assert(!animator.playsSound(repeating,position,1));animator.startAnimation(animation,false,0,6);animator.mTime=1;assert(animator.playsSound(repeating,position,1));animator.mTime=2;assert(!animator.playsSound(repeating,position,1));
    assert(animation->getSound(1)->isNoting(3)&&!animation->getSound(1)->isNoting(7));
    for(int index:{-1,3}){bool caught=false;try{animation->getSound(index);}catch(const std::out_of_range&){caught=true;}assert(caught);}
    std::fill(valid.begin(),valid.end(),0);assert(u32(animation->getSound(0)->mSoundID)==0x12345678);
    animator.removeAnimation();assert(!animator.mSoundAnimation);animator.startAnimation(none.animation(),true,0,0);assert(animator.mLoopSoundIndex==-1);animator.removeAnimation();
}
int main(int argc,char** argv){
    assert(argc==2);checkMalformed();NativeDiscArchiveLoader loader(std::filesystem::u8path(argv[1]));
    alignas(32) u8 heapBytes[262144]{};JKRExpHeap root(heapBytes,sizeof(heapBytes),nullptr,false);JKRHeap::sRootHeap=&root;root.becomeCurrentHeap();
    const auto initial=root.getTotalFreeSize();auto* heap=JKRExpHeap::create(131072,&root,false);assert(heap);
    std::vector<std::tuple<JAUSoundAnimation*,u32,float>> retained;std::size_t files=0,events=0,archives=0;
    JAISoundHandles handles(nullptr,0);JAUSoundAnimator animator(&handles);
    for(const char* path:{"/ObjectData/MarioAnime.arc","/ObjectData/LuigiAnime.arc","/ObjectData/Kinopio.arc","/ObjectData/Tico.arc","/ObjectData/Rosetta.arc","/ObjectData/DemoMario.arc","/ObjectData/MarioFace.arc"}){
        bool exists=false;for(const auto& file:loader.files())if(file.path==path)exists=true;if(!exists)continue;
        auto archive=loader.mount(path);std::size_t count=0;
        for(const auto& file:archive->files())if(file.path.size()>=4&&file.path.substr(file.path.size()-4)==".bas"){
            auto* bytes=archive->get(file.path);const auto size=archive->sizeOf(bytes);std::vector<u8> copy(bytes,bytes+size);BinaryView reference(copy);
            NativeOwnedBas* owner=nullptr;try{owner=new(heap,0) NativeOwnedBas(bytes,size);}catch(const std::exception& error){std::fprintf(stderr,"BAS %s:%s: %s\n",path,file.path.c_str(),error.what());return 10;}
            auto* animation=owner->animation();const auto sounds=reference.u16(0);assert(animation->getNumSounds()==sounds&&animation->mNumSounds==sounds&&animation->mControl);
            for(unsigned i=0;i<sounds;++i){auto at=8+i*32;const auto* sound=animation->getSound(i);
                assert(u32(sound->getSoundID())==reference.u32(at)&&sound->mNoteOnTime==reference.f32(at+4)&&sound->mNoteOffTime==reference.f32(at+8)&&sound->mBasePitch==reference.f32(at+12));
                assert(sound->mFlags==reference.u32(at+16)&&sound->mBaseVolume==reference.u8(at+20)&&sound->mPitchDelta==s8(reference.u8(at+21))&&sound->mPlayTime==reference.u8(at+22)&&sound->mBasePan==reference.u8(at+23));
                assert(sound->mVolumeDelta==s8(reference.u8(at+24))&&sound->mRepeatInterval==reference.u8(at+25)&&sound->_1A==s8(reference.u8(at+26))&&sound->_1C==reference.u32(at+28));
                for(float time:{sound->mNoteOnTime-.25f,sound->mNoteOnTime,sound->mNoteOnTime+.25f}){
                    unsigned start=0,end=0;while(start<sounds&&reference.f32(12+start*32)<time)++start;while(end<sounds&&reference.f32(12+end*32)<=time)++end;
                    assert(animation->getStartSoundIndex(time)==start&&animation->getEndSoundIndex(time)==end);
                }
                assert(animator.getSoundID(sound,TVec3f(0,0,0),1)==reference.u32(at));
            }
            animator.startAnimation(animation,false,0,100000);assert(animator.mSoundAnimation==animation&&animator.mLoopSoundIndex==0);
            animator.startAnimation(animation,true,0,100000);assert(animator.mLoopSoundIndex==int(sounds)-1);animator.removeAnimation();
            if(sounds)retained.emplace_back(animation,reference.u32(8),reference.f32(12));
            assert(std::memcmp(bytes,copy.data(),size)==0);events+=sounds;++count;++files;
        }
        assert(loader.unmount(path));archive.reset();std::printf("BAS archive %s: %zu resources\n",path,count);++archives;
    }
    assert(files>=39&&events>0);for(const auto& entry:retained){auto* animation=std::get<0>(entry);assert(u32(animation->getSound(0)->mSoundID)==std::get<1>(entry)&&animation->getSound(0)->mNoteOnTime==std::get<2>(entry));}
    const auto before=heap->getTotalFreeSize();auto bad=syntheticBas();bad.resize(9);bool rejected=false;try{new(heap,0) NativeOwnedBas(bad.data(),bad.size());}catch(const std::exception&){rejected=true;}assert(rejected&&heap->getTotalFreeSize()==before&&heap->check());
    heap->freeAll();assert(heap->mDisposerList.getNumLinks()==0&&heap->check());new(heap,0) NativeOwnedBas(syntheticBas().data(),104);heap->do_destroy();assert(root.getTotalFreeSize()==initial&&root.check());
    std::printf("PASS native BAS conversion: %zu archives, %zu resources, %zu sound events and original animator loop boundaries/start/remove\n",archives,files,events);
    std::puts("PASS BAS source lifetime, flags/parameters/signed values, empty and malformed resources, constructor rollback and heap disposal (zero playback handles; audible output not tested)");
}
