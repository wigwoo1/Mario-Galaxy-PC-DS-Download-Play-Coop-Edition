#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>
#include <cstring>
#include <memory>
#include <Native/GpuResources.hpp>

namespace Native {
// CPU-side register state, including nested display lists. Draw commands still
// require a renderer/vertex backend and are rejected rather than skipped.
class GxCommandState {
public:
    struct DestinationAlpha { bool enabled; std::uint8_t alpha; };
    DestinationAlpha destinationAlpha()const{return {(bp_[0x42]&256)!=0,std::uint8_t(bp_[0x42]&255)};}
    struct SyncEvent {enum class Kind {Token,Finish};Kind kind;std::uint16_t token;std::uint64_t sequence;};
    struct CopyOperation {
        std::array<std::uint32_t,256> registers;
        std::uint64_t sequence=0;
        unsigned left()const{return registers[0x49]&1023;}
        unsigned top()const{return (registers[0x49]>>10)&1023;}
        unsigned width()const{return (registers[0x4a]&1023)+1;}
        unsigned height()const{return ((registers[0x4a]>>10)&1023)+1;}
        std::uint32_t destinationAddress()const{return (registers[0x4b]&0xffffff)<<5;}
        unsigned strideBytes()const{return (registers[0x4d]&1023)<<5;}
        bool display()const{return (registers[0x52]&(1u<<14))!=0;}
        bool clear()const{return (registers[0x52]&(1u<<11))!=0;}
        bool verticalScale()const{return (registers[0x52]&(1u<<10))!=0;}
        unsigned gamma()const{return (registers[0x52]>>7)&3;}
        unsigned fieldMode()const{return (registers[0x52]>>12)&3;}
    };
private:
    std::vector<CopyOperation> copies_;
    std::vector<SyncEvent> syncEvents_;std::uint64_t commandSequence_=0;
    std::array<std::uint32_t,256> bp_{};
    std::array<bool,256> bpValid_{};
    std::array<std::uint32_t,256> cp_{};
    bool cpConfigured_=false;
    std::uint64_t vertexCacheGeneration_=0;
    std::array<std::uint32_t,0x1100> xf_{};
    std::array<bool,0x1100> xfValid_{};
    std::array<std::array<std::uint32_t,8>,2> tevColors_{};
    struct PaletteStorage {std::vector<std::uint8_t> bytes=std::vector<std::uint8_t>(0x80000);std::vector<bool> valid=std::vector<bool>(0x80000);};
    std::shared_ptr<PaletteStorage> palette_=std::make_shared<PaletteStorage>();

public:
    GxCommandState() { bp_[0xfe] = 0xffffff; }
    const std::vector<SyncEvent>& pendingSyncEvents()const{return syncEvents_;}
    void clearPendingSyncEvents(){syncEvents_.clear();}
    const std::vector<CopyOperation>& pendingCopies()const{return copies_;}
    void clearPendingCopies(){copies_.clear();}
    std::uint32_t bp(unsigned reg) const { return bp_.at(reg); }
    std::uint32_t cp(unsigned reg) const { return cp_.at(reg); }
    bool cpConfigured() const {return cpConfigured_;}
    std::uint64_t vertexCacheGeneration() const {return vertexCacheGeneration_;}
    std::uint32_t xf(unsigned reg) const { return xf_.at(reg); }
    bool xfLoaded(unsigned reg)const{return xfValid_.at(reg);}
    std::array<int,4> scissorRectangle()const{
        for(unsigned reg:{0x20u,0x21u,0x59u})if(!bpValid_[reg])throw std::runtime_error("GX scissor state not completely loaded");
        const int ox=2*int(bp_[0x59]&1023),oy=2*int((bp_[0x59]>>10)&1023);
        return {int((bp_[0x20]>>12)&2047)-ox,int(bp_[0x20]&2047)-oy,int((bp_[0x21]>>12)&2047)-ox+1,int(bp_[0x21]&2047)-oy+1};
    }
    // Recover screen bounds and normalized depth from original XF viewport words.
    std::array<float,6> viewport()const{
        std::array<float,6> v{};
        for(unsigned i=0;i<6;++i){if(!xfValid_[0x101a+i])throw std::runtime_error("GX viewport not completely loaded");auto bits=xf_[0x101a+i];std::memcpy(&v[i],&bits,4);if(!std::isfinite(v[i]))throw std::invalid_argument("Nonfinite GX viewport word");}
        return {v[3]-v[0]-342,v[4]+v[1]-342,2*v[0],-2*v[1],(v[5]-v[2])/16777216.f,v[5]/16777216.f};
    }
    std::array<std::array<float,4>,4> projectionMatrix()const{
        std::array<float,6> p{};
        for(unsigned i=0;i<7;++i)if(!xfValid_[0x1020+i])throw std::runtime_error("GX projection not completely loaded");
        const auto type=xf_[0x1026];if(type>1)throw std::invalid_argument("GX projection type");
        for(unsigned i=0;i<6;++i){auto bits=xf_[0x1020+i];std::memcpy(&p[i],&bits,4);if(!std::isfinite(p[i]))throw std::invalid_argument("Nonfinite GX projection coefficient");}
        std::array<std::array<float,4>,4> m{};
        m[0][0]=p[0];m[1][1]=p[2];m[2][2]=p[4];m[2][3]=p[5];
        m[0][type?3:2]=p[1];m[1][type?3:2]=p[3];
        if(type)m[3][3]=1;else m[3][2]=-1;
        return m;
    }
    std::array<float,3> transformPosition(unsigned matrix,const std::array<float,3>& position)const{
        if(matrix>27 || matrix%3)throw std::invalid_argument("Invalid GX position matrix slot");
        for(float value:position)if(!std::isfinite(value))throw std::invalid_argument("Nonfinite GX input position");
        std::array<float,3> result{};
        for(unsigned row=0;row<3;++row){for(unsigned col=0;col<4;++col){float value;auto address=matrix*4+row*4+col;if(!xfValid_[address])throw std::runtime_error("Position matrix is not completely loaded");auto bits=xf_[address];std::memcpy(&value,&bits,4);if(!std::isfinite(value))throw std::invalid_argument("Nonfinite XF position matrix");result[row]+=value*(col==3?1.f:position[col]);}if(!std::isfinite(result[row]))throw std::overflow_error("GX transformed position overflow");}
        return result;
    }
    std::array<float,3> transformNormal(unsigned matrix,const std::array<float,3>& normal)const{
        if(matrix>27 || matrix%3)throw std::invalid_argument("Invalid GX normal matrix slot");std::array<float,3> result{};
        for(unsigned row=0;row<3;++row)for(unsigned col=0;col<3;++col){unsigned address=0x400+matrix*3+row*3+col;if(!xfValid_[address])throw std::runtime_error("Normal matrix is not loaded");float value;auto bits=xf_[address];std::memcpy(&value,&bits,4);if(!std::isfinite(value)||!std::isfinite(normal[col]))throw std::invalid_argument("Nonfinite normal transform");result[row]+=value*normal[col];}
        for(float value:result)if(!std::isfinite(value))throw std::overflow_error("Normal transform overflow");return result;
    }
    // Evaluates an explicitly selected texture matrix against the texgen input.
    // Selection/source generation and dual-transform sequencing belong to texgen.
    std::array<float,3> transformTexture(unsigned id,const std::array<float,3>& input,bool projective,bool post=false)const{
        for(float v:input)if(!std::isfinite(v))throw std::invalid_argument("Nonfinite texgen input");
        if(!post&&id==60)return {input[0],input[1],projective?input[2]:1.f};
        if((!post&&(id>57||id%3))||(post&&(id>=60||id%3)))throw std::invalid_argument("Invalid texture matrix index");
        unsigned base=post?0x500+id*4:id*4;
        std::array<float,3> result{0,0,1};
        unsigned rows=projective?3:2;
        for(unsigned row=0;row<rows;++row){
            result[row]=0;
            for(unsigned col=0;col<4;++col){
                unsigned address=base+row*4+col;
                if(!xfValid_[address])throw std::runtime_error("Texture matrix not completely loaded");
                float coefficient;auto bits=xf_[address];std::memcpy(&coefficient,&bits,4);
                if(!std::isfinite(coefficient))throw std::invalid_argument("Nonfinite texture matrix: id="+std::to_string(id)+" post="+std::to_string(post)+" XF="+std::to_string(address)+" bits="+std::to_string(bits));
                result[row]+=coefficient*(col==3?1.f:input[col]);
            }
            if(!std::isfinite(result[row]))throw std::overflow_error("Texture matrix result overflow");
        }
        return result;
    }
    struct LightingChannel {bool materialVertex,enabled,ambientVertex;unsigned lightMask,diffuse;bool attenuation,spot;};
    LightingChannel lightingChannel(unsigned channel,bool alpha=false)const{
        if(channel>=2)throw std::out_of_range("GX lighting channel");
        unsigned address=0x100e + channel+(alpha?2:0);
        if(!xfValid_[address])throw std::runtime_error("GX channel control not loaded");
        auto word=xf_[address];
        unsigned diffuse=(word>>7)&3;if(diffuse==3)throw std::invalid_argument("Invalid GX diffuse function");
        return {bool(word&1),bool(word&2),bool(word&64),((word>>2)&15)|((word>>7)&240),diffuse,bool(word&512),bool(word&1024)};
    }
    std::array<unsigned,4> lightingColor(unsigned channel,bool ambient=false)const{
        if(channel>=2)throw std::out_of_range("GX lighting color");
        unsigned address=(ambient?0x100a:0x100c)+channel;
        if(!xfValid_[address])throw std::runtime_error("GX lighting color not loaded");
        auto value=xf_[address];return {value>>24,(value>>16)&255,(value>>8)&255,value&255};
    }
    struct Light {std::array<unsigned,4> color;std::array<float,3> angular,distance,position,direction;};
    Light light(unsigned slot)const{
        if(slot>=8)throw std::out_of_range("GX light slot");
        unsigned base=0x600+slot*16;
        for(unsigned i=3;i<16;++i)if(!xfValid_[base+i])throw std::runtime_error("GX light object not fully loaded");
        auto value=xf_[base+3];Light result{{value>>24,(value>>16)&255,(value>>8)&255,value&255},{},{},{},{}};
        auto vector=[&](unsigned offset){std::array<float,3> v{};for(unsigned i=0;i<3;++i){auto bits=xf_[base+offset+i];std::memcpy(&v[i],&bits,4);if(!std::isfinite(v[i]))throw std::invalid_argument("Nonfinite GX light data");}return v;};
        result.angular=vector(4);result.distance=vector(7);result.position=vector(10);result.direction=vector(13);return result;
    }
    struct CopyClear {std::array<std::uint8_t,4> rgba;std::uint32_t depth;};
    CopyClear copyClear()const{return {{{std::uint8_t(bp_[0x4f]),std::uint8_t(bp_[0x50]>>8),std::uint8_t(bp_[0x50]),std::uint8_t(bp_[0x4f]>>8)}},bp_[0x51]};}
    struct CopyFilter {std::array<std::array<unsigned,2>,12> samples;std::array<unsigned,7> coefficients;};
    CopyFilter copyFilter()const{
        CopyFilter result{};
        for(unsigned i=0;i<12;i++){const auto packed=bp_[1+i/3]>>((i%3)*8);result.samples[i]={packed&15,(packed>>4)&15};}
        for(unsigned i=0;i<7;i++)result.coefficients[i]=(bp_[i<4?0x53:0x54]>>((i<4?i:i-4)*6))&63;
        return result;
    }
    unsigned tevStageCount() const { return ((bp_[0]>>10)&15)+1; }
    struct DepthTexture {unsigned bias,type,operation;};
    DepthTexture depthTexture() const {return {bp_[0xf4],bp_[0xf5]&3,(bp_[0xf5]>>2)&3};}
    struct AlphaCompare {unsigned firstReference,secondReference,first,second,operation;};
    AlphaCompare alphaCompare() const {
        const auto value=bp_[0xf3];
        return {value&255,(value>>8)&255,(value>>16)&7,(value>>19)&7,(value>>22)&3};
    }
    struct TevCombiner {
        std::array<unsigned,4> inputs; // A, B, C, D selectors; not channel values.
        unsigned bias,scale,destination;
        bool subtract,clamp;
    };
    struct TevStage {
        TevCombiner color,alpha;
        unsigned rasterSwap,textureSwap;
    };
    struct TevOrder {unsigned texture,coordinate,channel;bool enabled;};
    struct TevKonst {unsigned color,alpha;};
    TevKonst tevKonst(unsigned stage)const{
        if(stage>=16)throw std::out_of_range("GX TEV constant selection");
        const auto value=bp_[0xf6+stage/2]>>((stage&1)*10);
        return {(value>>4)&31,(value>>9)&31};
    }
    std::array<unsigned,4> tevSwapTable(unsigned table) const {
        if(table>=4)throw std::out_of_range("GX TEV swap table");
        const auto rg=bp_[0xf6+table*2],ba=bp_[0xf7+table*2];
        return {rg&3,(rg>>2)&3,ba&3,(ba>>2)&3};
    }
    struct TevIndirect {
        unsigned stage,format,bias,alpha,matrix,wrapS,wrapT;
        bool unmodifiedLod,addPrevious;
    };
    TevIndirect tevIndirect(unsigned stage) const {
        if(stage>=16)throw std::out_of_range("GX indirect TEV stage");
        const auto value=bp_[0x10+stage];
        return {value&3,(value>>2)&3,(value>>4)&7,(value>>7)&3,(value>>9)&15,
                (value>>13)&7,(value>>16)&7,bool(value&(1u<<19)),bool(value&(1u<<20))};
    }
    TevOrder tevOrder(unsigned stage) const {
        if(stage>=16)throw std::out_of_range("GX TEV stage order");
        const unsigned value=bp_[0x28+stage/2]>>((stage&1)*12);
        return {value&7,(value>>3)&7,(value>>7)&7,bool(value&64)};
    }
    TevStage tevStage(unsigned stage) const {
        if(stage>=16)throw std::out_of_range("GX TEV stage");
        const auto color=bp_[0xc0+stage*2],alpha=bp_[0xc1+stage*2];
        auto decode=[](std::uint32_t value,std::array<unsigned,4> inputs){
            // Bias 3 encodes comparison operations; retain its raw value so
            // a future shader can distinguish these from add/subtract modes.
            return TevCombiner{inputs,(value>>16)&3,(value>>20)&3,(value>>22)&3,
                               bool(value&(1u<<18)),bool(value&(1u<<19))};
        };
        return {decode(color,{(color>>12)&15,(color>>8)&15,(color>>4)&15,color&15}),
                decode(alpha,{(alpha>>13)&7,(alpha>>10)&7,(alpha>>7)&7,(alpha>>4)&7}),
                alpha&3,(alpha>>2)&3};
    }
    std::array<int,4> tevColor(unsigned id,bool constant=false) const {
        if(id>=4)throw std::out_of_range("GX TEV color register");
        const auto ra=tevColors_[constant?1:0][id*2],bg=tevColors_[constant?1:0][id*2+1];
        auto channel=[&](unsigned value){value&=2047;return !constant && (value&1024) ? int(value)-2048 : int(value);};
        return {channel(ra),channel(bg>>12),channel(bg),channel(ra>>12)};
    }
    // Integer inputs for a TEV shader, before combiner arithmetic. Reserved
    // selectors resolve to zero, matching established GX shader behavior.
    // Fraction values: dolphin-emu/dolphin VideoCommon/PixelShaderGen.cpp.
    std::array<int,4> tevKonstValue(unsigned stage)const{
        const auto selection=tevKonst(stage);
        static constexpr int fractions[]={255,223,191,159,128,96,64,32};
        std::array<int,4> result{};
        if(selection.color<8)result[0]=result[1]=result[2]=fractions[selection.color];
        else if(selection.color>=12 && selection.color<16){
            auto color=tevColor(selection.color-12,true);
            std::copy_n(color.begin(),3,result.begin());
        }else if(selection.color>=16){
            int channel=tevColor(selection.color&3,true)[(selection.color-16)/4];
            result[0]=result[1]=result[2]=channel;
        }
        if(selection.alpha<8)result[3]=fractions[selection.alpha];
        else if(selection.alpha>=16)result[3]=tevColor(selection.alpha&3,true)[(selection.alpha-16)/4];
        return result;
    }
    struct PositionFormat {unsigned components,type,fraction,descriptor;};
    PositionFormat positionFormat(unsigned vat) const {
        if(vat>=8)throw std::out_of_range("GX vertex format slot");
        const auto value=cp_[0x70+vat];
        const unsigned type=(value>>1)&7;
        if(type>4)throw std::runtime_error("Invalid GX position component type");
        return {2+(value&1),type,(value>>4)&31,(cp_[0x50]>>9)&3};
    }
    std::array<float,3> indexedPosition(unsigned vat,unsigned index,const GpuResources& resources) const {
        const auto format=positionFormat(vat);
        if(format.descriptor<2)throw std::runtime_error("GX position is not indexed");
        if(index>(format.descriptor==2 ? 255u : 65535u))throw std::out_of_range("GX position index width");
        const unsigned width=format.type<2 ? 1 : format.type<4 ? 2 : 4;
        const std::uint64_t address=std::uint64_t(cp_[0xa0])+std::uint64_t(index)*(cp_[0xb0]&255);
        if(address>UINT32_MAX)throw std::out_of_range("GX position address overflow");
        const auto* bytes=resources.resolve(std::uint32_t(address),format.components*width);
        return decodePositionValues(vat,format,bytes);
    }
    std::array<float,3> directPosition(unsigned vat,const std::uint8_t* bytes,std::size_t size)const{
        const auto format=positionFormat(vat);
        if(format.descriptor!=1)throw std::runtime_error("GX position is not direct");
        unsigned width=format.type<2?1:format.type<4?2:4;
        if(!bytes||size<format.components*width)throw std::length_error("Truncated direct GX position");
        return decodePositionValues(vat,format,bytes);
    }
    std::array<float,3> decodePositionValues(unsigned vat,PositionFormat format,const std::uint8_t* bytes)const{
        const unsigned width=format.type<2?1:format.type<4?2:4;
        std::array<float,3> result{};
        for(unsigned c=0;c<format.components;++c){
            std::uint32_t bits=0;
            for(unsigned b=0;b<width;++b)bits=(bits<<8)|bytes[c*width+b];
            float value;
            if(format.type==4)std::memcpy(&value,&bits,4);
            else {
                const auto integer=format.type==1 ? (bits&128 ? int(bits)-256 : int(bits)) :
                    format.type==3 ? (bits&32768 ? int(bits)-65536 : int(bits)) : int(bits);
                const unsigned fraction=format.type<2 && !(cp_[0x70+vat]&(1u<<30)) ? 0 : format.fraction;
                value=std::ldexp(float(integer),-int(fraction));
            }
            if(!std::isfinite(value))throw std::runtime_error("Nonfinite GX position");
            result[c]=value;
        }
        return result;
    }
    std::array<float,3> indexedScalarAttribute(unsigned array,unsigned components,unsigned type,unsigned fraction,unsigned index,const GpuResources& resources)const{
        if(array>11 || components>3 || !components || type>4 || fraction>31)throw std::invalid_argument("GX scalar attribute format");
        unsigned width=type<2?1:type<4?2:4;std::uint64_t address=std::uint64_t(cp_[0xa0+array])+std::uint64_t(index)*(cp_[0xb0+array]&255);
        if(address>UINT32_MAX)throw std::out_of_range("GX attribute address overflow");auto* bytes=resources.resolve(std::uint32_t(address),components*width);return directScalarAttribute(components,type,fraction,bytes,components*width);
    }
    std::array<float,3> directScalarAttribute(unsigned components,unsigned type,unsigned fraction,const std::uint8_t* bytes,std::size_t size)const{
        if(components>3||!components||type>4||fraction>31)throw std::invalid_argument("GX direct scalar format");
        unsigned width=type<2?1:type<4?2:4;
        if(!bytes||size<components*width)throw std::length_error("Truncated direct scalar attribute");
        std::array<float,3> result{};
        for(unsigned c=0;c<components;++c){std::uint32_t bits=0;for(unsigned b=0;b<width;++b)bits=(bits<<8)|bytes[c*width+b];
            if(type==4)std::memcpy(&result[c],&bits,4);
            else {int value=type==1?(bits&128?int(bits)-256:int(bits)):type==3?(bits&32768?int(bits)-65536:int(bits)):int(bits);result[c]=std::ldexp(float(value),-int(fraction));}
            if(!std::isfinite(result[c]))throw std::invalid_argument("Nonfinite direct GX attribute");
        }return result;
    }
    unsigned directNormalSize(unsigned vat)const{
        if(vat>=8)throw std::out_of_range("GX normal VAT");
        if(((cp_[0x50]>>11)&3)!=1)throw std::invalid_argument("GX normal is not direct");
        auto word=cp_[0x70+vat];if(word&(1u<<9))throw std::invalid_argument("NBT normal requires separate decoder");
        unsigned type=(word>>10)&7;if(type!=1&&type!=3&&type!=4)throw std::invalid_argument("GX normal type");
        return 3*(type==1?1:type==3?2:4);
    }
    std::array<float,3> directNormal(unsigned vat,const std::uint8_t* bytes,std::size_t size)const{
        auto length=directNormalSize(vat);if(size<length)throw std::length_error("Truncated direct GX normal");
        unsigned type=(cp_[0x70+vat]>>10)&7;
        return directScalarAttribute(3,type,type==1?6:type==3?14:0,bytes,size);
    }
    std::array<float,3> indexedNormal(unsigned vat,unsigned index,const GpuResources& resources)const{
        if(vat>=8)throw std::out_of_range("GX normal VAT");auto word=cp_[0x70+vat];if(word&(1u<<9))throw std::invalid_argument("NBT normal requires separate decoder");unsigned type=(word>>10)&7;
        if(type!=1 && type!=3 && type!=4)throw std::invalid_argument("GX normal type");return indexedScalarAttribute(1,3,type,type==1?6:type==3?14:0,index,resources);
    }
    struct TexCoordFormat {unsigned components,type,fraction,descriptor;};
    TexCoordFormat texCoordFormat(unsigned vat,unsigned channel)const{
        if(vat>=8||channel>=8)throw std::out_of_range("GX texture VAT/channel");
        unsigned word,shift,fraction;
        if(channel==0){word=cp_[0x70+vat];shift=21;fraction=(word>>25)&31;}
        else if(channel<=3){word=cp_[0x80+vat];shift=(channel-1)*9;fraction=(word>>(shift+4))&31;}
        else if(channel==4){word=cp_[0x80+vat];shift=27;fraction=cp_[0x90+vat]&31;}
        else{word=cp_[0x90+vat];shift=5+(channel-5)*9;fraction=(word>>(shift+4))&31;}
        unsigned type=(word>>(shift+1))&7;if(type>4)throw std::invalid_argument("GX texture scalar type");
        return {1+((word>>shift)&1),type,fraction,(cp_[0x60]>>(channel*2))&3};
    }
    unsigned directTexCoordSize(unsigned vat,unsigned channel)const{
        auto f=texCoordFormat(vat,channel);if(f.descriptor!=1)throw std::invalid_argument("GX texture coordinate is not direct");
        return f.components*(f.type<2?1:f.type<4?2:4);
    }
    std::array<float,3> directTexCoord(unsigned vat,unsigned channel,const std::uint8_t* bytes,std::size_t size)const{
        directTexCoordSize(vat,channel);auto f=texCoordFormat(vat,channel);return directScalarAttribute(f.components,f.type,f.fraction,bytes,size);
    }
    std::array<float,3> indexedTexCoord(unsigned vat,unsigned channel,unsigned index,const GpuResources& resources)const{
        auto f=texCoordFormat(vat,channel);if(f.descriptor<2||index>(f.descriptor==2?255u:65535u))throw std::invalid_argument("GX texture coordinate index");
        return indexedScalarAttribute(4+channel,f.components,f.type,f.fraction,index,resources);
    }
    std::array<float,3> indexedTexCoord0(unsigned vat,unsigned index,const GpuResources& resources)const{return indexedTexCoord(vat,0,index,resources);}
    std::array<unsigned,4> indexedColor(unsigned vat,unsigned channel,unsigned index,const GpuResources& resources)const{
        if(vat>=8||channel>=2)throw std::out_of_range("GX vertex color format");
        unsigned descriptor=(cp_[0x50]>>(13+channel*2))&3;
        if(descriptor<2||index>(descriptor==2?255u:65535u))throw std::invalid_argument("Invalid indexed color descriptor/index");
        unsigned format=(cp_[0x70+vat]>>(14+channel*4))&7;
        if(format>5)throw std::invalid_argument("Invalid GX color encoding");
        static const unsigned widths[]={2,3,4,2,3,4};
        unsigned attribute=2+channel;
        uint64_t address=uint64_t(cp_[0xa0+attribute])+uint64_t(index)*(cp_[0xb0+attribute]&255);
        if(address>UINT32_MAX)throw std::out_of_range("GX color address overflow");
        auto* bytes=resources.resolve(uint32_t(address),widths[format]);
        return decodeColorValues(format,bytes);
    }
    unsigned directColorSize(unsigned vat,unsigned channel)const{
        if(vat>=8||channel>=2)throw std::out_of_range("GX vertex color format");
        unsigned descriptor=(cp_[0x50]>>(13+channel*2))&3;
        if(descriptor!=1)throw std::invalid_argument("GX color is not direct");
        unsigned format=(cp_[0x70+vat]>>(14+channel*4))&7;
        if(format>5)throw std::invalid_argument("Invalid GX color encoding");
        static const unsigned widths[]={2,3,4,2,3,4};return widths[format];
    }
    std::array<unsigned,4> directColor(unsigned vat,unsigned channel,const std::uint8_t* bytes,std::size_t size)const{
        unsigned length=directColorSize(vat,channel);
        if(!bytes||size<length)throw std::length_error("Truncated direct GX color");
        return decodeColorValues((cp_[0x70+vat]>>(14+channel*4))&7,bytes);
    }
    static std::array<unsigned,4> decodeColorValues(unsigned format,const std::uint8_t* bytes){
        static const unsigned widths[]={2,3,4,2,3,4};
        auto expand=[](unsigned value,unsigned bits){return bits==4?value*17:bits==5?(value<<3)|(value>>2):(value<<2)|(value>>4);};
        if(format==1||format==2)return {bytes[0],bytes[1],bytes[2],255};
        if(format==5)return {bytes[0],bytes[1],bytes[2],bytes[3]};
        unsigned packed=0;for(unsigned i=0;i<widths[format];++i)packed=(packed<<8)|bytes[i];
        if(format==0)return {expand((packed>>11)&31,5),expand((packed>>5)&63,6),expand(packed&31,5),255};
        unsigned bits=format==3?4:6,mask=(1u<<bits)-1;
        return {expand((packed>>(bits*3))&mask,bits),expand((packed>>(bits*2))&mask,bits),expand((packed>>bits)&mask,bits),expand(packed&mask,bits)};
    }
    struct Sampler {unsigned wrapS,wrapT,minFilter,magFilter;};
    Sampler sampler(unsigned slot)const{
        if(slot>=8)throw std::out_of_range("GX sampler slot");
        auto mode=bp_[(slot<4?0x80:0xa0)+(slot&3)];unsigned encoded=(mode>>5)&7;
        static const unsigned filters[]={0,2,4,0,1,3,5,0};
        if(encoded==3 || encoded==7 || (mode&3)>2 || ((mode>>2)&3)>2)throw std::invalid_argument("Unsupported GX sampler fields");
        return {mode&3,(mode>>2)&3,filters[encoded],(mode>>4)&1};
    }
    struct Texture { std::uint32_t width,height,format,address; };
    Texture texture(unsigned slot) const {
        if (slot >= 8) throw std::out_of_range("GX texture slot");
        const unsigned bank = slot < 4 ? 0x80 : 0xa0;
        const unsigned index = slot & 3;
        const auto attr = bp_[bank + 8 + index];
        return {(attr & 1023) + 1, ((attr >> 10) & 1023) + 1,
                (attr >> 20) & 15, bp_[bank + 20 + index] << 5};
    }
    std::vector<std::uint8_t> palette(unsigned slot, unsigned maxEntries) const {
        if (slot >= 8) throw std::out_of_range("GX palette slot");
        if (!maxEntries || maxEntries > 16384) throw std::out_of_range("GX palette entry count");
        const auto reg = bp_[(slot < 4 ? 0x98 : 0xb8) + (slot & 3)];
        const unsigned start = (reg & 1023) << 9;
        unsigned length = 0;
        while (length < maxEntries*2 && start+length < palette_->bytes.size() && palette_->valid[start+length]) ++length;
        length &= ~1u;
        if (!length) throw std::runtime_error("GX texture palette has not been transferred");
        return {palette_->bytes.begin()+start,palette_->bytes.begin()+start+length};
    }
    unsigned paletteFormat(unsigned slot) const {
        if (slot >= 8) throw std::out_of_range("GX palette slot");
        return (bp_[(slot < 4 ? 0x98 : 0xb8)+(slot & 3)] >> 10) & 3;
    }
    void consume(const std::uint8_t* data, std::size_t size, const GpuResources* resources = nullptr) {
        std::size_t budget=64*1024*1024;
        consumeList(data,size,resources,0,budget);
    }
    std::size_t vertexPayloadSize(unsigned vat) const {
        if(vat>=8)throw std::out_of_range("GX vertex format slot");
        const unsigned low=cp_[0x50],high=cp_[0x60];
        std::size_t size=0;
        for(unsigned i=0;i<9;++i)size+=(low>>i)&1;
        const auto position=positionFormat(vat);
        if(!position.descriptor)throw std::invalid_argument("Missing GX position descriptor");
        const unsigned positionWidth=position.type<2?1:position.type<4?2:4;
        size+=position.descriptor==1?position.components*positionWidth:position.descriptor==2?1:2;
        for(unsigned attr=10;attr<=20;++attr){
            const unsigned descriptor=attr<=12?(low>>(11+2*(attr-10)))&3:(high>>(2*(attr-13)))&3;
            if(!descriptor)continue;
            if(descriptor>=2){
                const unsigned indices=attr==10&&((cp_[0x70+vat]&(1u<<9))!=0)&&((cp_[0x70+vat]&(1u<<31))!=0)?3:1;
                size+=(descriptor==2?1:2)*indices;
            }else if(attr==10){
                const auto word=cp_[0x70+vat];
                const unsigned type=(word>>10)&7;
                if(type!=1&&type!=3&&type!=4)throw std::invalid_argument("GX normal type");
                size+=((word&(1u<<9))?9u:3u)*(type==1?1:type==3?2:4);
            }else if(attr<=12)size+=directColorSize(vat,attr-11);
            else size+=directTexCoordSize(vat,attr-13);
        }
        return size;
    }
private:
    void consumeList(const std::uint8_t* data,std::size_t size,const GpuResources* resources,unsigned depth,std::size_t& budget) {
        if(depth>32 || size>budget)throw std::length_error("GX display-list execution limit");
        budget-=size;
        if (!data && size) throw std::invalid_argument("Null GX command bytes");
        // A malformed/unsupported list must not partially change register state.
        auto next = *this;
        std::size_t pos = 0;
        auto read = [&](unsigned count) {
            if (count > size - pos) throw std::length_error("Truncated GX command");
            std::uint32_t value = 0;
            for (unsigned i=0;i<count;++i) value = (value<<8) | data[pos++];
            return value;
        };
        while (pos < size) {
            const auto op = read(1);
            if(next.commandSequence_==UINT64_MAX)throw std::overflow_error("GX command sequence overflow");++next.commandSequence_;
            if (op == 0) continue;
            if (op == 0x48) {++next.vertexCacheGeneration_;continue;}
            const unsigned primitive=op&0xf8;
            if(primitive==0x80||primitive==0x90||primitive==0x98||primitive==0xa0||primitive==0xa8||primitive==0xb0||primitive==0xb8){
                const auto count=read(2);
                const auto stride=next.vertexPayloadSize(op&7);
                if(!stride||count>(size-pos)/stride)throw std::length_error("Truncated GX primitive payload");
                pos+=std::size_t(count)*stride;
                continue;
            }
            if (op == 0x61) {
                const auto value = read(4);
                const unsigned reg = value >> 24;
                const auto mask = next.bp_[0xfe];
                if(mask==0xffffff)next.bpValid_[reg]=true;
                next.bp_[reg] = (next.bp_[reg] & ~mask) | (value & mask);
                if (reg != 0xfe) next.bp_[0xfe] = 0xffffff;
                if(reg>=0xe0 && reg<=0xe7){
                    const unsigned bank=(next.bp_[reg]>>23)&1;
                    next.tevColors_[bank][reg-0xe0]=next.bp_[reg];
                }
                if(reg==0x52){
                    if(next.copies_.size()>=65536)throw std::length_error("GX copy operation limit");
                    next.copies_.push_back({next.bp_,next.commandSequence_});
                }
                if(reg==0x48 || (reg==0x45 && (next.bp_[reg]&255)==2)){
                    if(next.syncEvents_.size()>=65536)throw std::length_error("GX sync event limit");
                    next.syncEvents_.push_back({reg==0x48?SyncEvent::Kind::Token:SyncEvent::Kind::Finish,std::uint16_t(reg==0x48?next.bp_[reg]:0),next.commandSequence_});
                }
                if (reg == 0x65) {
                    if (!resources) throw std::runtime_error("GX palette transfer requires resource storage");
                    const auto dest = (next.bp_[reg] & 1023) << 9;
                    const auto bytes = ((next.bp_[reg] >> 10) & 2047) * 32;
                    if (!bytes || bytes > next.palette_->bytes.size()-dest)
                        throw std::out_of_range("GX palette transfer extent");
                    const auto* source = resources->resolve(next.bp_[0x64] << 5,bytes);
                    if(!next.palette_.unique())next.palette_=std::make_shared<PaletteStorage>(*next.palette_);
                    std::copy(source,source+bytes,next.palette_->bytes.begin()+dest);
                    std::fill(next.palette_->valid.begin()+dest,next.palette_->valid.begin()+dest+bytes,true);
                }
            } else if (op == 8) {
                const unsigned reg = read(1);
                next.cp_[reg] = read(4); next.cpConfigured_=true;
            } else if (op == 0x10) {
                const auto count = read(2) + 1;
                const auto start = read(2);
                if (start >= next.xf_.size() || count > next.xf_.size()-start)
                    throw std::out_of_range("GX XF register range");
                for (unsigned i=0;i<count;++i) {next.xf_[start+i] = read(4);next.xfValid_[start+i]=true;}
            } else if (op == 0x20 || op == 0x28 || op == 0x30 || op == 0x38) {
                if (!resources) throw std::runtime_error("Indexed XF command requires resource storage");
                const auto index = read(2);
                const auto descriptor = read(2);
                const auto count = (descriptor >> 12) + 1;
                const auto destination = descriptor & 4095;
                const unsigned array = 12 + (op-0x20)/8;
                const std::uint64_t address = std::uint64_t(next.cp_[0xa0+array]) +
                    std::uint64_t(index) * (next.cp_[0xb0+array] & 255);
                if (address > UINT32_MAX || count > next.xf_.size()-destination)
                    throw std::out_of_range("Indexed XF resource range");
                const auto* source = resources->resolve(std::uint32_t(address),count*4);
                for (unsigned i=0;i<count;++i) {
                    std::uint32_t value = 0;
                    for (unsigned j=0;j<4;++j) value = (value<<8) | source[i*4+j];
                    next.xf_[destination+i] = value; next.xfValid_[destination+i]=true;
                }
            } else if(op==0x40) {
                const auto address=read(4),length=read(4);
                if(!resources)throw std::runtime_error("GX display-list call requires resource storage");
                const auto* list=resources->resolve(address,length);
                next.consumeList(list,length,resources,depth+1,budget);
            } else {
                throw std::runtime_error("GX command requires unimplemented draw or indexed backend");
            }
        }
        *this = next;
    }
};
}


























