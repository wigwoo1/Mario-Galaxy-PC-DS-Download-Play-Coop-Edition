#pragma once
#include "assets.hpp"
#include <cmath>
#include <map>

class BinaryView {
    const std::vector<uint8_t>* bytes;
    size_t base, length;
public:
    explicit BinaryView(const std::vector<uint8_t>& b) : bytes(&b), base(0), length(b.size()) {}
    BinaryView(const std::vector<uint8_t>& b,size_t start,size_t size) : bytes(&b),base(start),length(size) { extent(start,size,b.size()); }
    size_t size() const { return length; }
    uint8_t u8(size_t p) const { extent(p,1,length); return (*bytes)[base+p]; }
    uint16_t u16(size_t p) const { extent(p,2,length); return uint16_t(u8(p))<<8|u8(p+1); }
    uint32_t u32(size_t p) const { extent(p,4,length); return be32(bytes->data()+base+p); }
    float f32(size_t p) const { uint32_t bits=u32(p); float f; std::memcpy(&f,&bits,4); assetCheck(std::isfinite(f),"Non-finite model value"); return f; }
    BinaryView sub(size_t p,size_t n) const { extent(p,n,length); return BinaryView(*bytes,base+p,n); }
    std::string tag(size_t p) const { extent(p,4,length); return std::string(reinterpret_cast<const char*>(bytes->data()+base+p),4); }
};
struct Pixel { uint8_t r,g,b,a; };
struct TextureLevel {uint32_t width,height;std::vector<Pixel> pixels;};
struct TextureImage { uint32_t width,height,format; std::vector<Pixel> pixels; uint8_t wrapS=1,wrapT=1,minFilter=1,magFilter=1; std::vector<TextureLevel> mipmaps; int8_t minLod=0,maxLod=127;int16_t lodBias=0;bool commandLod=false;float commandMinLod=0,commandMaxLod=0,commandLodBias=0; };
inline size_t gxEncodedSize(uint32_t w,uint32_t h,uint32_t format){
    uint32_t bw=4,bh=4,bytes=32;switch(format){case 0:case 8:bw=bh=8;break;case 1:case 2:case 9:bw=8;break;case 3:case 4:case 5:case 10:break;case 6:bytes=64;break;case 14:bw=bh=8;break;default:throw std::runtime_error("Invalid mip texture format");}return size_t((w+bw-1)/bw)*((h+bh-1)/bh)*bytes;
}
inline uint8_t expand5(unsigned v) { return uint8_t((v<<3)|(v>>2)); }
inline uint8_t expand6(unsigned v) { return uint8_t((v<<2)|(v>>4)); }
inline Pixel rgb565(uint16_t v) { return {expand5(v>>11),expand6((v>>5)&63),expand5(v&31),255}; }
inline Pixel rgb5a3(uint16_t v) {
    if (v&0x8000) return {expand5((v>>10)&31),expand5((v>>5)&31),expand5(v&31),255};
    unsigned a=(v>>12)&7;
    return {uint8_t(((v>>8)&15)*17),uint8_t(((v>>4)&15)*17),uint8_t((v&15)*17),uint8_t((a<<5)|(a<<2)|(a>>1))};
}
inline TextureImage decodeGxTexture(BinaryView input,uint32_t w,uint32_t h,uint32_t format,
                                    size_t offset,BinaryView palette,uint32_t paletteCount=0,uint32_t paletteFormat=0) {
    assetCheck(w>0 && h>0 && w<=8192 && h<=8192 && uint64_t(w)*h<=16777216,"Invalid or oversized texture dimensions");
    uint32_t bw=4,bh=4,blockBytes=32;
    switch(format) {
    case 0: case 8: bw=8;bh=8;break;
    case 1: case 2: case 9: bw=8;bh=4;break;
    case 3: case 4: case 5: case 10: break;
    case 6: blockBytes=64;break;
    case 14: bw=8;bh=8;break;
    default: throw std::runtime_error("Unsupported GX texture format: "+std::to_string(format));
    }
    uint64_t bytes=uint64_t((w+bw-1)/bw)*((h+bh-1)/bh)*blockBytes;
    extent(offset,bytes,input.size());
    const bool indexed=format==8||format==9||format==10;
    if(indexed) { assetCheck(paletteCount>0 && paletteFormat<=2,"Invalid texture palette"); extent(0,uint64_t(paletteCount)*2,palette.size()); }
    auto pal=[&](uint32_t i) {
        assetCheck(i<paletteCount,"Texture palette index outside table"); uint16_t v=palette.u16(size_t(i)*2);
        if(paletteFormat==0) return Pixel{uint8_t(v),uint8_t(v),uint8_t(v),uint8_t(v>>8)};
        return paletteFormat==1?rgb565(v):rgb5a3(v);
    };
    TextureImage image{w,h,format,std::vector<Pixel>(size_t(w)*h)};
    auto put=[&](uint32_t x,uint32_t y,Pixel c) { if(x<w&&y<h) image.pixels[size_t(y)*w+x]=c; };
    for(uint32_t y=0;y<h;y+=bh) for(uint32_t x=0;x<w;x+=bw,offset+=blockBytes) {
        if(format==14) {
            for(unsigned sub=0;sub<4;++sub) {
                size_t p=offset+sub*8; uint16_t a=input.u16(p),b=input.u16(p+2);
                Pixel colors[4]={rgb565(a),rgb565(b),{}, {}};
                for(int c=0;c<3;++c) {
                    auto channel=[&](Pixel z) { return c==0?z.r:(c==1?z.g:z.b); };
                    uint8_t va=channel(colors[0]),vb=channel(colors[1]);
                    uint8_t c2=a>b?uint8_t((5*va+3*vb)>>3):uint8_t((va+vb)/2);
                    uint8_t c3=a>b?uint8_t((3*va+5*vb)>>3):uint8_t((va+vb)/2);
                    if(c==0) {colors[2].r=c2;colors[3].r=c3;}
                    if(c==1) {colors[2].g=c2;colors[3].g=c3;}
                    if(c==2) {colors[2].b=c2;colors[3].b=c3;}
                }
                colors[2].a=255;colors[3].a=a>b?255:0;
                for(unsigned iy=0;iy<4;++iy) for(unsigned ix=0;ix<4;++ix)
                    put(x+(sub%2)*4+ix,y+(sub/2)*4+iy,colors[(input.u8(p+4+iy)>>(6-ix*2))&3]);
            }
            continue;
        }
        for(uint32_t iy=0;iy<bh;++iy) for(uint32_t ix=0;ix<bw;++ix) {
            if(x+ix>=w||y+iy>=h) continue;
            size_t i=iy*bw+ix; Pixel c{};
            switch(format) {
            case 0: {uint8_t v=uint8_t(((input.u8(offset+i/2)>>(i%2?0:4))&15)*17);c={v,v,v,v};break;}
            case 1: {uint8_t v=input.u8(offset+i);c={v,v,v,v};break;}
            case 2: {uint8_t v=input.u8(offset+i),k=uint8_t((v&15)*17);c={k,k,k,uint8_t((v>>4)*17)};break;}
            case 3: {uint8_t a=input.u8(offset+i*2),v=input.u8(offset+i*2+1);c={v,v,v,a};break;}
            case 4:c=rgb565(input.u16(offset+i*2));break;
            case 5:c=rgb5a3(input.u16(offset+i*2));break;
            case 6:c={input.u8(offset+i*2+1),input.u8(offset+32+i*2),input.u8(offset+33+i*2),input.u8(offset+i*2)};break;
            case 8:c=pal((input.u8(offset+i/2)>>(i%2?0:4))&15);break;
            case 9:c=pal(input.u8(offset+i));break;
            case 10:c=pal(input.u16(offset+i*2)&0x3fff);break;
            }
            put(x+ix,y+iy,c);
        }
    }
    return image;
}
inline TextureImage decodeBti(BinaryView view,size_t header=0) {
    extent(header,32,view.size());
    uint32_t paletteCount=view.u16(header+10); size_t paletteOffset=header+uint64_t(view.u32(header+12));
    auto palette=paletteCount?view.sub(paletteOffset,size_t(paletteCount)*2):view.sub(0,0);
    auto image=decodeGxTexture(view,view.u16(header+2),view.u16(header+4),view.u8(header),header+uint64_t(view.u32(header+28)),palette,paletteCount,view.u8(header+9));
    image.wrapS=view.u8(header+6);image.wrapT=view.u8(header+7);image.minFilter=view.u8(header+20);image.magFilter=view.u8(header+21);assetCheck(image.wrapS<=2&&image.wrapT<=2,"Invalid BTI texture wrap mode");assetCheck(image.minFilter<=5&&image.magFilter<=1,"Invalid BTI texture filter");
    image.minLod=int8_t(view.u8(header+22));image.maxLod=int8_t(view.u8(header+23));image.lodBias=int16_t(view.u16(header+26));assetCheck(image.minLod<=image.maxLod,"Invalid BTI LOD range");
    size_t count=std::max<size_t>(1,view.u8(header+24)),maximum=1;for(uint32_t dimension=std::max(image.width,image.height);dimension>1;dimension>>=1)++maximum;assetCheck(count<=maximum,"BTI mip count exceeds dimensions");
    size_t offset=header+uint64_t(view.u32(header+28));uint32_t w=image.width,h=image.height;for(size_t level=1;level<count;++level){offset+=gxEncodedSize(w,h,image.format);w=std::max(1u,w/2);h=std::max(1u,h/2);auto mip=decodeGxTexture(view,w,h,image.format,offset,palette,paletteCount,view.u8(header+9));image.mipmaps.push_back({w,h,std::move(mip.pixels)});}return image;
}
inline std::vector<TextureImage> decodeTpl(BinaryView view) {
    assetCheck(view.size()>=12 && view.u32(0)==0x0020af30,"Invalid TPL header");
    uint32_t count=view.u32(4);size_t table=view.u32(8);
    assetCheck(count>0&&count<=4096,"Invalid TPL texture count");extent(table,uint64_t(count)*8,view.size());
    std::vector<TextureImage> images;
    for(uint32_t i=0;i<count;++i) {
        size_t header=view.u32(table+i*8),ph=view.u32(table+i*8+4); extent(header,36,view.size());
        uint32_t pc=0,pf=0; auto palette=view.sub(0,0);
        if(ph) {extent(ph,12,view.size());pc=view.u16(ph);pf=view.u32(ph+4);palette=view.sub(view.u32(ph+8),size_t(pc)*2);}
        images.push_back(decodeGxTexture(view,view.u16(header+2),view.u16(header),view.u32(header+4),view.u32(header+8),palette,pc,pf));
    }
    return images;
}
inline void mipmapTests(){
    std::vector<uint8_t> data(128);data[3]=data[5]=4;data[24]=3;data[31]=32;std::fill(data.begin()+32,data.begin()+64,0x11);std::fill(data.begin()+64,data.begin()+96,0x22);std::fill(data.begin()+96,data.end(),0x33);
    auto image=decodeBti(BinaryView(data));assetCheck(image.mipmaps.size()==2&&image.mipmaps[0].width==2&&image.mipmaps[1].width==1&&image.pixels[0].r==17&&image.mipmaps[0].pixels[0].r==34&&image.mipmaps[1].pixels[0].r==51,"BTI mip offsets/dimensions/pixels failed");
    bool rejected=false;data.pop_back();try{decodeBti(BinaryView(data));}catch(const std::exception&){rejected=true;}assetCheck(rejected,"Truncated BTI mip accepted");data.resize(128);data[24]=4;rejected=false;try{decodeBti(BinaryView(data));}catch(const std::exception&){rejected=true;}assetCheck(rejected,"Excess BTI mip count accepted");
}
inline std::vector<uint8_t> textureBmp(const TextureImage& image) {
    // 24-bit BMP preview composited onto a checkerboard; decoder retains real alpha.
    size_t stride=(size_t(image.width)*3+3)&~size_t(3);
    std::vector<uint8_t> bmp(54+stride*image.height,0);
    auto le=[&](size_t p,uint32_t v) {for(int i=0;i<4;++i) bmp[p+i]=uint8_t(v>>(i*8));};
    bmp[0]='B';bmp[1]='M';le(2,uint32_t(bmp.size()));le(10,54);le(14,40);le(18,image.width);le(22,image.height);
    bmp[26]=1;bmp[28]=24;le(34,uint32_t(stride*image.height));
    for(uint32_t y=0;y<image.height;++y) for(uint32_t x=0;x<image.width;++x) {
        Pixel p=image.pixels[size_t(y)*image.width+x];unsigned bg=((x/16+y/16)%2)?56:32;
        size_t out=54+(image.height-1-y)*stride+x*3;
        bmp[out]=uint8_t((p.b*p.a+bg*(255-p.a)+127)/255);
        bmp[out+1]=uint8_t((p.g*p.a+bg*(255-p.a)+127)/255);
        bmp[out+2]=uint8_t((p.r*p.a+bg*(255-p.a)+127)/255);
    }
    return bmp;
}

struct ModelInfo {
    std::vector<std::pair<std::string,size_t>> sections;
    uint32_t vertices=0,joints=0,shapes=0,materials=0;
    std::vector<std::array<float,3>> positions;
    std::vector<TextureImage> textures;
};
inline ModelInfo decodeModel(BinaryView view) {
    assetCheck(view.size()>=32 && view.tag(0)=="J3D2" && (view.tag(4)=="bdl4"||view.tag(4)=="bmd3"),"Unsupported J3D model");
    assetCheck(view.u32(8)==view.size(),"J3D file length mismatch");
    uint32_t count=view.u32(12);assetCheck(count>0&&count<=64,"Invalid J3D section count");
    ModelInfo model;std::map<std::string,BinaryView> sections;
    size_t p=32;
    for(uint32_t i=0;i<count;++i) {
        extent(p,8,view.size());size_t size=view.u32(p+4);assetCheck(size>=8,"Invalid J3D section length");
        auto section=view.sub(p,size);std::string tag=section.tag(0);
        assetCheck(sections.emplace(tag,section).second,"Duplicate J3D section");
        model.sections.push_back({tag,size});p+=size;
    }
    assetCheck(p==view.size(),"Unaccounted J3D file data");
    for(const char* required:{"INF1","VTX1","JNT1","SHP1","MAT3","TEX1"}) assetCheck(sections.count(required),"Missing required J3D section");
    auto info=sections.at("INF1");model.vertices=info.u32(16);
    model.joints=sections.at("JNT1").u16(8);model.shapes=sections.at("SHP1").u16(8);model.materials=sections.at("MAT3").u16(8);
    assetCheck(model.vertices<=1000000,"Excessive J3D vertex count");
    auto vertex=sections.at("VTX1");size_t formatOffset=vertex.u32(8),posOffset=vertex.u32(12);
    bool found=false,terminated=false;
    for(size_t i=0;i<32;++i) {
        size_t f=formatOffset+i*16;uint32_t attr=vertex.u32(f);
        if(attr==255) {terminated=true;break;}
        if(attr!=9) continue;
        assetCheck(!found,"Duplicate J3D position format");found=true;
        uint32_t components=vertex.u32(f+4),type=vertex.u32(f+8),fraction=vertex.u8(f+12);
        assetCheck(components<=1&&type<=4&&fraction<=31,"Unsupported J3D position format");
        size_t n=components?3:2,element=type<=1?1:(type<=3?2:4);
        extent(posOffset,uint64_t(model.vertices)*n*element,vertex.size());
        auto component=[&](size_t q) {
            if(type==4) return vertex.f32(q);
            double value=type==0?vertex.u8(q):type==1?int8_t(vertex.u8(q)):type==2?vertex.u16(q):int16_t(vertex.u16(q));
            return float(std::ldexp(value,-int(fraction)));
        };
        for(uint32_t v=0;v<model.vertices;++v) {
            size_t q=posOffset+size_t(v)*n*element;
            model.positions.push_back({component(q),component(q+element),n==3?component(q+element*2):0});
        }
    }
    assetCheck(found&&terminated,"Missing J3D position format or terminator");
    auto texture=sections.at("TEX1");uint32_t num=texture.u16(8);size_t headers=texture.u32(12);
    extent(headers,uint64_t(num)*32,texture.size());
    for(uint32_t i=0;i<num;++i) model.textures.push_back(decodeBti(texture,headers+i*32));
    return model;
}

