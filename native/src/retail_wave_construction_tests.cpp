#include <JSystem/JKernel/JKRSolidHeap.hpp>
#include <JSystem/JKernel/JKRExpHeap.hpp>
#include <JSystem/JAudio2/JASBasicInst.hpp>
#include <cassert>
#include <JSystem/JUtility/JUTException.hpp>
#include <cstdio>
#include <cstdarg>
extern "C" void JUTWarningConsole_f(const char*,...){assert(false);}
extern "C" void JUTReportConsole_f(const char*,...){}
extern "C" void JUTWarningConsole(const char* message){std::fputs(message,stderr);assert(false);}
extern "C" void JUTReportConsole(const char* message){std::fputs(message,stderr);}
void JUTException::panic_f(const char* file,int line,const char* format,...){std::fprintf(stderr,"%s:%d ",file,line);va_list args;va_start(args,format);std::vfprintf(stderr,format,args);va_end(args);assert(false);}
#include <JSystem/JAudio2/JASBasicBank.hpp>
#include <JSystem/JAudio2/JASHeapCtrl.hpp>
#include "native_dvd_port.hpp"
extern JKRSolidHeap* JASDram; JKRHeap* bankHeap=nullptr; unsigned waveBanksBuilt=0; unsigned constructed=0,instrumentCount=0,envelopesRun=0;unsigned long long evaluated=0,accepted=0;
#include <JSystem/JAudio2/JAUAudioArcInterpreter.hpp>
#include <JSystem/JAudio2/JASWSParser.hpp>
#include <stdexcept>
#include <cmath>
#include <JSystem/JAudio2/JASBNKParser.hpp>
#include <fstream>
#include <vector>
#include <cassert>
#include <cstdio>
struct Inspector:JAUAudioArcInterpreter{const u8* base;size_t length;unsigned ws=0,banks=0,bst=0,names=0,other=0,waveGroups=0,waves=0,bankVersions[2]{},oscillators=0,envelopePoints=0;void check(const void* p,u32 n=1){auto* b=static_cast<const u8*>(p);assert(b>=base&&size_t(b-base)<=length&&n<=length-size_t(b-base));}void readWS(u32,const void* p,u32)override{
 check(p,sizeof(JASWSParser::THeader));auto* h=static_cast<const JASWSParser::THeader*>(p);u32 size=nativeWSU32(&h->mSize);check(p,size);
 auto bound=[&](const void* ptr,size_t n){auto* b=static_cast<const u8*>(ptr);auto* start=static_cast<const u8*>(p);if(b<start||size_t(b-start)>size||n>size-size_t(b-start))throw std::runtime_error("Retail WS extent invalid");};
 auto* group=h->mCtrlGroupOffset.ptr(h);bound(group,12);u32 groups=nativeWSU32(&group->mGroupCount);bound(group,12+size_t(groups)*4);auto* archives=h->mArchiveBankOffset.ptr(h);bound(archives,8+size_t(groups)*4);
 for(u32 g=0;g<groups;++g){auto* scene=group->mCtrlSceneOffsets[g].ptr(h);bound(scene,16);auto* ctrl=scene->mCtrlOffset.ptr(h);bound(ctrl,8);u32 count=nativeWSU32(&ctrl->mWaveCount);bound(ctrl,8+size_t(count)*4);auto* arc=archives->mArchiveOffsets[g].ptr(h);bound(arc,0x74+size_t(count)*4);if(!std::memchr(arc->mFileName,0,0x70))throw std::runtime_error("Wave filename unterminated");for(u32 j=0;j<count;++j){auto* wave=arc->mWaveOffsets[j].ptr(h);bound(wave,sizeof(*wave));auto* id=ctrl->mCtrlWaveOffsets[j].ptr(h);bound(id,4);float rate=nativeWSFloat(&wave->mSampleRate);if(!std::isfinite(rate)||rate<=0||wave->mFormat>=8)throw std::runtime_error("Invalid retail wave format/rate");++waves;}++waveGroups;}std::printf("Construct waveform bank %u\n",ws);auto* waveBank=JASWSParser::createWaveBank(p,bankHeap);if(!waveBank)throw std::runtime_error("Wave bank construction failed");if(!bankHeap->check())throw std::runtime_error("Wave heap corrupted");++waveBanksBuilt;++ws;
}void readBNK(u32,const void* p)override{check(p,16);auto* h=static_cast<const JASBNKParser::TFileHeader*>(p);u32 version=nativeBNKU32(&h->mVersion),size=nativeBNKU32(&h->mSize);check(p,size);if(version>1||size<32)throw std::runtime_error("Unsupported retail bank header");for(u32 tag:{0x454e5654u,0x4f534354u,0x4c495354u}){auto* chunk=JASBNKParser::Ver1::findChunk(p,tag);if(!chunk){std::fprintf(stderr,"Missing bank %u tag %08x size %u\n",JASBNKParser::getBankNumber(p),tag,size);auto* dump=static_cast<const u8*>(p);for(unsigned k=0;k<80;++k)std::fprintf(stderr,"%02x%s",dump[k],k%16==15?"\n":" ");throw std::runtime_error("Missing required bank chunk");}check(chunk,8+nativeBNKU32(&chunk->mSize));}if(JASBNKParser::Ver1::findChunk(p,0x58585858))throw std::runtime_error("Unexpected chunk");auto* env=static_cast<JASBNKParser::Ver1::TEnvtChunk*>(JASBNKParser::Ver1::findChunk(p,0x454e5654));
auto* oscs=static_cast<JASBNKParser::Ver1::TOscChunk*>(JASBNKParser::Ver1::findChunk(p,0x4f534354));
u32 envSize=nativeBNKU32(&env->mSize),oscSize=nativeBNKU32(&oscs->mSize),count=nativeBNKU32(&oscs->mCount);
if(oscSize<4||size_t(count)*sizeof(JASBNKParser::Ver1::TOsc)>oscSize-4)throw std::runtime_error("Oscillator count exceeds chunk");
for(u32 k=0;k<count;++k){const auto& osc=oscs->mOsc[k];for(const void* field:{static_cast<const void*>(&osc._08),static_cast<const void*>(&osc.mScale),static_cast<const void*>(&osc._18)}){u32 bits=nativeBNKU32(field);float value;std::memcpy(&value,&bits,4);if(!std::isfinite(value))throw std::runtime_error("Nonfinite oscillator parameter");}
for(u32 offset:{nativeBNKU32(&osc.mTableOffset),nativeBNKU32(&osc._10)}){bool terminal=false;for(size_t pos=offset;pos+6<=envSize;pos+=6){const auto* b=env->mData+pos;int command=static_cast<int16_t>((u16(b[0])<<8)|b[1]);++envelopePoints;if(command>10){terminal=true;break;}}if(!terminal)throw std::runtime_error("Envelope lacks bounded terminal point");}++oscillators;}++bankVersions[version];auto number=JASBNKParser::getBankNumber(p);if(number>=256)throw std::runtime_error("Retail bank number out of range");auto* bank=JASBNKParser::Ver1::createBasicBank(p,bankHeap);if(!bank)throw std::runtime_error("Retail bank construction failed");++constructed;for(int program=0;program<240;++program){auto* inst=bank->getInst(program);if(!inst)continue;++instrumentCount;bool envelopeChecked=false;for(int key=0;key<128;++key)for(int velocity=0;velocity<128;++velocity){JASInstParam param;++evaluated;if(!inst->getParam(key,velocity,&param))continue;++accepted;for(float value:{param.mVolume,param.mPitch,param.mPan,param.mFxMix,param.mDolby})if(!std::isfinite(value))throw std::runtime_error("Nonfinite evaluated instrument parameter");if(param.mNumOscillators>2||param.mChannelType!=JASChannel::CH_WAVE)throw std::runtime_error("Invalid evaluated instrument channel");if(param.mNumOscillators&&!param.mOscillatorData)throw std::runtime_error("Missing oscillator array");if(!envelopeChecked){for(unsigned oscIndex=0;oscIndex<param.mNumOscillators;++oscIndex){auto* data=param.mOscillatorData[oscIndex];if(!data)continue;JASOscillator osc;osc.initStart(data);if(param.mDirectRelease)osc.setDirectRelease(param.mDirectRelease);for(int tick=0;tick<512&&!osc.isStop();++tick){osc.incCounter(1.0f);if(!std::isfinite(osc.getValue()))throw std::runtime_error("Nonfinite attack envelope");}osc.release();for(int tick=0;tick<2048&&!osc.isStop();++tick){osc.incCounter(1.0f);if(!std::isfinite(osc.getValue()))throw std::runtime_error("Nonfinite release envelope");}++envelopesRun;}envelopeChecked=true;}}}if(!bankHeap->check())throw std::runtime_error("Bank heap corrupted");++banks;}void readBSC(const void* p,u32 n)override{check(p,n);++other;}void readBST(const void* p,u32 n)override{check(p,n);assert(n==38656);++bst;}void readBSTN(const void* p,u32 n)override{check(p,n);assert(n==93920);++names;}void readBMS(u32,const void* p,u32 n)override{check(p,n);++other;}void readBMS_fromArchive(u32)override{++other;}void newVoiceBank(u32,u32)override{++other;}void newDynamicSeqBlock(u32)override{++other;}void readBSFT(const void* p)override{check(p);++other;}void readMaxSeCategory(int,int,int)override{++other;}void beginBNKList(u32,u32)override{++other;}void endBNKList()override{++other;}bool readCommandMore(u32)override{assert(false&&"Unknown retail command");return false;}};
int main(int argc,char** argv){if(argc!=3)return 2;std::setvbuf(stdout,nullptr,_IONBF,0);if(!nativeBindDvd(argv[2])){std::fputs("DVD bind failed\n",stderr);return 3;}alignas(32) static u8 arena[8*1024*1024];JKRSolidHeap parent(arena,sizeof(arena),nullptr,false);JKRHeap::sRootHeap=&parent;bankHeap=JKRExpHeap::create(7*1024*1024,&parent,false);assert(bankHeap);bankHeap->becomeCurrentHeap();JKRHeap::sSystemHeap=bankHeap;JASKernel::sSystemHeap=bankHeap;JASDram=&parent;std::ifstream f(argv[1],std::ios::binary);std::vector<u8> bytes((std::istreambuf_iterator<char>(f)),{});assert(bytes.size()==596320);Inspector i;i.base=bytes.data();i.length=bytes.size();assert(!i.parse(nullptr));assert(i.parse(bytes.data()));assert(i.bst==1&&i.names==1&&i.ws&&i.banks);assert(i.mReadPtr<=bytes.data()+bytes.size());std::printf("PASS original retail BAA dispatch: %u waveform systems, %u banks, BST/BSTN and %u other commands\n",i.ws,i.banks,i.other);std::printf("PASS retail waveform traversal: %u archive groups, %u wave records\n",i.waveGroups,i.waves);std::printf("PASS retail oscillator traversal: %u oscillators, %u envelope point visits\n",i.oscillators,i.envelopePoints);std::printf("PASS actual retail bank construction: %u banks\n",constructed);std::printf("PASS retail instrument evaluation: %u instruments, %llu key/velocity pairs, %llu accepted notes\n",instrumentCount,evaluated,accepted);std::printf("PASS retail attack/release execution: %u oscillator runs\n",envelopesRun);std::printf("PASS actual retail waveform banks: %u\n",waveBanksBuilt);std::printf("Retail banks: version0=%u version1=%u\n",i.bankVersions[0],i.bankVersions[1]);}












