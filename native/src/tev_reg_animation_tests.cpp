#define main originalHeapTestEntry
#include "unit_heap_live_tests.cpp"
#undef main
#include "native_owned_tev_reg.hpp"
#include "assets.hpp"
#include "native_model_blocks.hpp"
#include <JSystem/J3DGraphAnimator/J3DMaterialAttach.hpp>
#include <algorithm>
#include <cmath>

static double referenceSample(BinaryView block,size_t track,size_t values,float frame){
    const unsigned count=block.u16(track),offset=block.u16(track+2),type=block.u16(track+4);
    auto read=[&](size_t i){return double(s16(block.u16(values+(offset+i)*2)));};
    if(!count)return 0;if(count==1)return read(0);
    const size_t stride=type?4:3;
    if(frame<=read(0))return read(1);
    if(frame>=read((count-1)*stride))return read((count-1)*stride+1);
    unsigned k=0;while(k+1<count && frame>=read((k+1)*stride))++k;
    const size_t a=k*stride,b=a+stride;
    const double duration=read(b)-read(a),t=(frame-read(a))/duration;
    return (2*t*t*t-3*t*t+1)*read(a+1)+(t*t*t-2*t*t+t)*duration*read(a+(type?3:2))
          +(-2*t*t*t+3*t*t)*read(b+1)+(t*t*t-t*t)*duration*read(b+2);
}
static void word(std::vector<u8>& b,size_t at,u32 v){for(unsigned i=0;i<4;++i)b.at(at+i)=u8(v>>(24-i*8));}
static void half(std::vector<u8>& b,size_t at,u16 v){b.at(at)=u8(v>>8);b.at(at+1)=u8(v);}
static void reject(const std::vector<u8>& bytes){bool caught=false;try{NativeTevRegAnimation bad(bytes.data(),bytes.size());}catch(const std::exception&){caught=true;}assert(caught);}

static void verifySynthetic(){
    std::vector<u8> b(512);std::memcpy(b.data(),"J3D1brk1",8);word(b,8,u32(b.size()));word(b,12,1);
    const size_t base=32;std::memcpy(b.data()+base,"TRK1",4);word(b,base+4,u32(b.size()-base));half(b,base+10,10);
    half(b,base+12,1);half(b,base+14,1);
    for(unsigned bank=0;bank<2;++bank){
        const size_t table=0x58+bank*28;word(b,base+0x20+bank*4,u32(table));word(b,base+0x28+bank*4,0x90+bank*2);word(b,base+0x30+bank*4,0x94);
        for(unsigned ch=0;ch<4;++ch){
            half(b,base+table+ch*6,2);half(b,base+table+ch*6+4,u16(bank));
            unsigned channel=bank*4+ch;const size_t values=0xa0+channel*16;
            half(b,base+0x10+channel*2,bank?8:6);word(b,base+0x38+channel*4,u32(values));
            const s16 shared[]={0,-100,0,10,100,0},split[]={0,0,0,0,10,255,0,0};
            for(unsigned i=0;i<(bank?8:6);++i)half(b,base+values+i*2,u16(bank?split[i]:shared[i]));
        }
    }
    half(b,base+0x94,1);half(b,base+0x98,0);half(b,base+0x9a,8);std::memcpy(b.data()+base+0x9c,"mat",4);
    NativeTevRegAnimation a(b.data(),b.size());a.mFrame=5;GXColorS10 c{};GXColor k{};a.getTevColorReg(0,&c);a.getTevKonstReg(0,&k);
    assert(c.r==0 && c.g==0 && c.b==0 && c.a==0 && k.r==127 && k.g==127 && k.b==127 && k.a==127);
    a.mFrame=-5;a.getTevColorReg(0,&c);a.getTevKonstReg(0,&k);assert(c.r==-100 && k.r==0);
    a.mFrame=20;a.getTevColorReg(0,&c);a.getTevKonstReg(0,&k);assert(c.r==100 && k.r==255);
    assert(__OSf32tos16(-40000)==-32768 && __OSf32tos16(40000)==32767 && __OSf32tos16(-2.9f)==-2);
    assert(__OSf32tou8(-2)==0 && __OSf32tou8(300)==255 && __OSf32tou8(2.9f)==2);
    auto bad=b;half(bad,base+0xa0+6,0);reject(bad);
    // Empty and constant tracks preserve the original sampler's branches.
    half(b,base+0x58,0);half(b,base+0x58+6,1);NativeTevRegAnimation simple(b.data(),b.size());simple.getTevColorReg(0,&c);assert(c.r==0 && c.g==0);
    std::vector<u8> empty(0x78);std::memcpy(empty.data(),"J3D1brk1",8);word(empty,8,u32(empty.size()));word(empty,12,1);std::memcpy(empty.data()+32,"TRK1",4);word(empty,36,0x58);
    NativeTevRegAnimation none(empty.data(),empty.size());assert(!none.mCRegUpdateMaterialNum && !none.mKRegUpdateMaterialNum);
    std::puts("PASS synthetic BRK shared/separate tangents, empty/constant tracks, endpoint sampling and host signed/unsigned quantization");
}

int main(int argc,char** argv){
    assert(argc==2);verifySynthetic();WiiAssets disc(std::filesystem::u8path(argv[1]));
    alignas(32) u8 arena[524288]{};JKRExpHeap root(arena,sizeof(arena),nullptr,false);
    JKRHeap::sRootHeap=&root;root.becomeCurrentHeap();const auto initial=root.getTotalFreeSize();
    auto* heap=JKRExpHeap::create(262144,&root,false);assert(heap);
    size_t files=0,archives=0,samples=0,matches=0;bool kinopio=false;std::vector<u8> example;
    std::vector<std::string> paths{"/ObjectData/Kinopio.arc"};
    for(const auto& f:disc.files)if(f.path.find("/ObjectData/")==0 && f.path.size()>4 && f.path.substr(f.path.size()-4)==".arc" && f.size<4*1024*1024 && f.path!=paths.front())paths.push_back(f.path);
    for(const auto& path:paths){
        Rarc archive(disc.readFile(path));size_t inArchive=0;
        for(const auto& f:archive.files){
            auto source=archive.readFile(f.path);
            if(source.size()<8 || std::memcmp(source.data(),"J3D1brk1",8))continue;
            const auto raw=source;BinaryView view(raw);auto block=view.sub(32,view.u32(36));
            NativeOwnedTevReg* owner=nullptr;
            try{owner=new(heap,0) NativeOwnedTevReg(source.data(),source.size());}
            catch(const std::exception& e){std::fprintf(stderr,"BRK %s:%s: %s\n",path.c_str(),f.path.c_str(),e.what());return 10;}
            auto* a=static_cast<J3DAnmTevRegKey*>(owner->animation());
            assert(source==raw && a->getKind()==5 && a->mFrameMax==s16(block.u16(10)) && a->mAttribute==block.u8(8));
            std::fill(source.begin(),source.end(),0xdd);source.clear();source.shrink_to_fit();
            for(unsigned bank=0;bank<2;++bank){
                const unsigned count=block.u16(12+bank*2);
                assert((bank?a->mKRegUpdateMaterialNum:a->mCRegUpdateMaterialNum)==count);
                auto& names=bank?a->mKRegUpdateMaterialName:a->mCRegUpdateMaterialName;
                for(unsigned i=0;i<count;++i){
                    const size_t table=block.u32(0x20+bank*4)+i*28;
                    assert((bank?a->mAnmKRegKeyTable[i].mColorId:a->mAnmCRegKeyTable[i].mColorId)==block.u8(table+24));
                    assert((bank?a->getKRegUpdateMaterialID(i):a->getCRegUpdateMaterialID(i))==block.u16(block.u32(0x28+bank*4)+i*2));
                    const size_t nt=block.u32(0x30+bank*4),name=nt+block.u16(nt+6+i*4);
                    assert(!std::strcmp(names.getName(i),reinterpret_cast<const char*>(raw.data()+32+name)));
                    for(float frame:{-5.f,0.f,.5f,1.5f,float(a->mFrameMax)/2,float(a->mFrameMax),float(a->mFrameMax)+10}){
                        a->mFrame=frame;GXColorS10 c{};GXColor k{};if(bank)a->getTevKonstReg(i,&k);else a->getTevColorReg(i,&c);
                        const int actual[]={bank?k.r:c.r,bank?k.g:c.g,bank?k.b:c.b,bank?k.a:c.a};
                        for(unsigned ch=0;ch<4;++ch){
                            const auto track=table+ch*6;double expected=referenceSample(block,track,block.u32(0x38+(bank*4+ch)*4),frame);
                            if(block.u16(track)>1)expected=std::clamp(expected,bank?0.:-1024.,bank?255.:1023.);
                            const int quantized=bank?int(u8(int(expected))):int(s16(int(expected)));
                            assert(std::abs(actual[ch]-quantized)<=1);++samples;
                        }
                    }
                }
            }
            // Match animation names to a real MAT3, then test missing names.
            for(const auto& modelFile:archive.files){
                auto model=archive.readFile(modelFile.path);if(model.size()<8 || std::memcmp(model.data(),"J3D2",4))continue;
                NativeModelBlocks blocks(model.data(),model.size());
                for(const auto& section:blocks.blocks){
                    if(std::memcmp(model.data()+section.offset,"MAT3",4))continue;
                    BinaryView mv(model);auto mat=mv.sub(section.offset,section.length);auto nt=mat.u32(20);
                    NativeNameTable storage(mat.sub(nt,mat.size()-nt));JUTNameTab names(storage.resource());
                    J3DMaterialTable table;table.mMaterialName=&names;table.mMaterialNum=names.mNameNum;a->searchUpdateMaterialID(&table);
                    for(unsigned bank=0;bank<2;++bank){auto& an=bank?a->mKRegUpdateMaterialName:a->mCRegUpdateMaterialName;unsigned count=bank?a->mKRegUpdateMaterialNum:a->mCRegUpdateMaterialNum;
                        for(unsigned i=0;i<count;++i){int id=names.getIndex(an.getName(i));assert((bank?a->getKRegUpdateMaterialID(i):a->getCRegUpdateMaterialID(i))==u16(id));if(id>=0)++matches;}}
                    names.setResource(nullptr);a->searchUpdateMaterialID(&table);
                    for(unsigned i=0;i<a->mCRegUpdateMaterialNum;++i)assert(!a->isValidCRegUpdateMaterialID(i));
                    for(unsigned i=0;i<a->mKRegUpdateMaterialNum;++i)assert(!a->isValidKRegUpdateMaterialID(i));
                }
            }
            if(path==paths.front() && f.path.find("colorchange")!=std::string::npos){kinopio=true;example=raw;}
            ++files;++inArchive;heap->freeAll();assert(heap->mDisposerList.getNumLinks()==0 && heap->check());
        }
        if(inArchive){++archives;std::printf("BRK %s: %zu resources\n",path.c_str(),inArchive);}
        if(archives>=12)break;
    }
    assert(kinopio && files>=12 && samples>0 && matches>0 && !example.empty());
    reject({});auto bad=example;bad.resize(39);reject(bad);
    bad=example;word(bad,8,39);reject(bad);bad=example;word(bad,36,0xffffffff);reject(bad);
    bad=example;word(bad,32+0x20,0xffffffff);reject(bad);
    BinaryView raw(example);auto block=raw.sub(32,raw.u32(36));const auto t=32+block.u32(0x20);
    bad=example;half(bad,t+4,2);reject(bad);bad=example;half(bad,t,65535);reject(bad);
    bad=example;bad.at(t+24)=4;reject(bad);bad=example;word(bad,32+0x30,0xffffffff);reject(bad);
    const auto before=heap->getTotalFreeSize();bool caught=false;try{new(heap,0) NativeOwnedTevReg(bad.data(),bad.size());}catch(const std::exception&){caught=true;}
    assert(caught && heap->getTotalFreeSize()==before && !heap->mDisposerList.getNumLinks());
    new(heap,0) NativeOwnedTevReg(example.data(),example.size());heap->do_destroy();assert(root.getTotalFreeSize()==initial && root.check());
    std::printf("PASS BRK: %zu archives, %zu resources, %zu original-sampler channel samples, %zu real material matches; Kinopio ColorChange, source lifetime, malformed resources, constructor rollback and heap disposal\n",archives,files,samples,matches);
}
