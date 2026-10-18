#pragma once
#include "graphics.hpp"
#include <optional>
struct NativeLayoutText{uint16_t bufferBytes=0,stringBytes=0,font=0;uint8_t position=0,alignment=0;std::array<uint32_t,2> colors{};std::array<float,2> fontSize{};float characterSpace=0,lineSpace=0;std::u16string text;};
struct NativeLayoutPane {
    std::optional<NativeLayoutText> text;std::string name,kind,userData;int parent=-1;uint8_t flags=0,base=0,alpha=255;
    std::array<float,3> translate{},rotate{};std::array<float,2> scale{},size{};
    std::array<float,4> inflation{};std::vector<std::pair<uint16_t,uint8_t>> windowFrames;uint16_t material=0;std::array<uint32_t,4> colors{};std::vector<std::array<float,8>> uv;
};
struct NativeLayoutTextureMap{uint16_t texture;uint8_t wrapS,wrapT;};
struct NativeLayoutMaterial{std::string name;uint32_t resourceBits=0;std::vector<NativeLayoutTextureMap> textures;std::vector<std::array<float,5>> textureSrt;std::vector<std::array<uint8_t,3>> coordinateGenerators;std::array<std::array<int16_t,4>,3> tevColors{};std::array<uint32_t,4> konstColors{};};
struct NativeLayoutResource{std::array<float,2> size{};uint8_t origin=0;uint16_t materialCount=0;std::vector<NativeLayoutPane> panes;std::vector<std::string> textureNames,fontNames;std::vector<NativeLayoutMaterial> materials;};
inline std::string layoutFixedName(BinaryView block,size_t offset,size_t length){extent(offset,length,block.size());std::string name;for(size_t i=0;i<length&&block.u8(offset+i);++i)name.push_back(char(block.u8(offset+i)));assetCheck(!name.empty(),"Empty layout pane name");return name;}
inline NativeLayoutResource decodeLayoutResource(BinaryView data){
    assetCheck(data.size()>=16&&data.tag(0)=="RLYT"&&data.u16(4)==0xFEFF&&data.u32(8)==data.size(),"Invalid layout resource header");
    size_t offset=data.u16(12);assetCheck(offset>=16&&offset<=data.size(),"Invalid layout resource header size");NativeLayoutResource result;std::vector<int> parents;int last=-1;bool foundSize=false,foundMaterials=false,foundTextures=false,foundFonts=false;
    for(unsigned i=0;i<data.u16(14);++i){extent(offset,8,data.size());auto length=data.u32(offset+4);assetCheck(length>=8,"Invalid layout block size");auto block=data.sub(offset,length);auto tag=block.tag(0);
        if(tag=="lyt1"){assetCheck(!foundSize&&length>=20&&block.u8(8)<=1,"Invalid layout size block");result.origin=block.u8(8);result.size={block.f32(12),block.f32(16)};assetCheck(result.size[0]>0&&result.size[1]>0,"Invalid layout dimensions");foundSize=true;}
        else if(tag=="txl1"){
            assetCheck(!foundTextures&&length>=12,"Invalid layout texture list");auto count=block.u16(8);assetCheck(count<=1024,"Excess layout textures");extent(12,uint64_t(count)*8,length);
            for(unsigned j=0;j<count;++j){auto entry=12+j*8;auto nameOffset=12+uint64_t(block.u32(entry));assetCheck(nameOffset>=12+uint64_t(count)*8&&nameOffset<length,"Invalid layout texture name offset");std::string name;for(size_t k=nameOffset;k<length&&block.u8(k);++k)name.push_back(char(block.u8(k)));assetCheck(nameOffset+name.size()<length,"Unterminated layout texture name");safeName(name);result.textureNames.push_back(name);}foundTextures=true;
        }
        else if(tag=="fnl1"){
            assetCheck(!foundFonts&&length>=12,"Invalid layout font list");auto count=block.u16(8);assetCheck(count<=1024,"Excess layout fonts");extent(12,uint64_t(count)*8,length);for(unsigned j=0;j<count;++j){auto nameOffset=12+uint64_t(block.u32(12+j*8));assetCheck(nameOffset>=12+uint64_t(count)*8&&nameOffset<length,"Invalid layout font name offset");std::string name;for(size_t k=nameOffset;k<length&&block.u8(k);++k)name.push_back(char(block.u8(k)));assetCheck(nameOffset+name.size()<length,"Unterminated layout font name");safeName(name);result.fontNames.push_back(name);}foundFonts=true;
        }
        else if(tag=="mat1"){assetCheck(!foundMaterials&&length>=12,"Invalid layout material list");result.materialCount=block.u16(8);extent(12,uint64_t(result.materialCount)*4,length);for(unsigned j=0;j<result.materialCount;++j){auto material=block.u32(12+j*4);assetCheck(material>=12+result.materialCount*4,"Layout material overlaps offset table");extent(material,64,length);size_t end=length;for(unsigned k=0;k<result.materialCount;++k)if(k!=j){auto other=block.u32(12+k*4);assetCheck(other!=material,"Duplicate layout material offsets");if(other>material)end=std::min(end,size_t(other));}auto data=block.sub(material,end-material);NativeLayoutMaterial decoded;decoded.name=layoutFixedName(data,0,20);decoded.resourceBits=data.u32(60);for(unsigned registerId=0;registerId<3;++registerId)for(unsigned component=0;component<4;++component)decoded.tevColors[registerId][component]=int16_t(data.u16(20+registerId*8+component*2));for(unsigned registerId=0;registerId<4;++registerId)decoded.konstColors[registerId]=data.u32(44+registerId*4);auto maps=decoded.resourceBits&15,srts=(decoded.resourceBits>>4)&15,coords=(decoded.resourceBits>>8)&15;assetCheck(maps<=8&&srts<=10&&coords<=8,"Unsupported layout material resource counts");size_t cursor=64;extent(cursor,maps*4+srts*20+coords*4,data.size());for(unsigned k=0;k<maps;++k){auto ws=data.u8(cursor+2),wt=data.u8(cursor+3);assetCheck(ws<=2&&wt<=2,"Invalid layout texture wrapping");decoded.textures.push_back({data.u16(cursor),ws,wt});cursor+=4;}for(unsigned k=0;k<srts;++k){std::array<float,5> srt;for(unsigned n=0;n<5;++n)srt[n]=data.f32(cursor+n*4);decoded.textureSrt.push_back(srt);cursor+=20;}for(unsigned k=0;k<coords;++k){decoded.coordinateGenerators.push_back({data.u8(cursor),data.u8(cursor+1),data.u8(cursor+2)});cursor+=4;}result.materials.push_back(std::move(decoded));}foundMaterials=true;}
        else if(tag=="pas1"){assetCheck(last>=0&&parents.size()<64,"Invalid layout hierarchy begin");parents.push_back(last);last=-1;}
        else if(tag=="pae1"){assetCheck(!parents.empty(),"Unbalanced layout hierarchy end");last=parents.back();parents.pop_back();}
        else if(tag=="pan1"||tag=="pic1"||tag=="txt1"||tag=="wnd1"||tag=="bnd1"){
            assetCheck(length>=76&&result.panes.size()<1024,"Invalid layout pane block");NativeLayoutPane pane;pane.kind=tag;pane.name=layoutFixedName(block,12,16);if(block.u8(28))pane.userData=layoutFixedName(block,28,8);pane.parent=parents.empty()?-1:parents.back();pane.flags=block.u8(8);pane.base=block.u8(9);pane.alpha=block.u8(10);assetCheck(pane.base<=8,"Invalid layout pane base position");
            for(unsigned j=0;j<3;++j){pane.translate[j]=block.f32(36+j*4);pane.rotate[j]=block.f32(48+j*4);}for(unsigned j=0;j<2;++j){pane.scale[j]=block.f32(60+j*4);pane.size[j]=block.f32(68+j*4);assetCheck(pane.size[j]>=0,"Negative layout pane dimensions");}
            if(tag=="pic1"){extent(76,20,length);for(unsigned j=0;j<4;++j)pane.colors[j]=block.u32(76+j*4);pane.material=block.u16(92);auto count=block.u8(94);assetCheck(count<=8,"Excess layout UV sets");extent(96,uint64_t(count)*32,length);for(unsigned j=0;j<count;++j){std::array<float,8> uv;for(unsigned k=0;k<8;++k)uv[k]=block.f32(96+j*32+k*4);pane.uv.push_back(uv);}}
            if(tag=="txt1"){
                extent(76,40,length);NativeLayoutText text;text.bufferBytes=block.u16(76);text.stringBytes=block.u16(78);pane.material=block.u16(80);text.font=block.u16(82);text.position=block.u8(84);text.alignment=block.u8(85);assetCheck(text.position<=8&&text.alignment<=3&&!(text.bufferBytes&1)&&!(text.stringBytes&1)&&text.stringBytes<=text.bufferBytes,"Invalid layout text buffer/alignment");auto stringOffset=block.u32(88);for(unsigned j=0;j<2;++j){text.colors[j]=block.u32(92+j*4);text.fontSize[j]=block.f32(100+j*4);assetCheck(text.fontSize[j]>0,"Invalid layout text font size");}text.characterSpace=block.f32(108);text.lineSpace=block.f32(112);
                if(text.stringBytes){assetCheck(stringOffset>=116,"Layout text overlaps pane header");extent(stringOffset,text.stringBytes,length);bool terminated=false;for(unsigned j=0;j<text.stringBytes/2;++j){auto unit=block.u16(stringOffset+j*2);if(!unit){terminated=true;break;}text.text.push_back(char16_t(unit));}assetCheck(terminated,"Unterminated layout UTF-16 text");}pane.text=std::move(text);
            }
            if(tag=="wnd1"){
                extent(76,28,length);for(unsigned j=0;j<4;++j)pane.inflation[j]=block.f32(76+j*4);auto count=block.u8(92);assetCheck(count<=8,"Excess layout window frames");auto content=block.u32(96),table=block.u32(100);assetCheck(content>=104,"Window content overlaps header");extent(content,20,length);
                for(unsigned j=0;j<4;++j)pane.colors[j]=block.u32(content+j*4);pane.material=block.u16(content+16);auto uvCount=block.u8(content+18);assetCheck(uvCount<=8,"Excess window UV sets");extent(content+20,uint64_t(uvCount)*32,length);for(unsigned j=0;j<uvCount;++j){std::array<float,8> uv;for(unsigned k=0;k<8;++k)uv[k]=block.f32(content+20+j*32+k*4);pane.uv.push_back(uv);}
                if(count){assetCheck(table>=104,"Window frame table overlaps header");extent(table,uint64_t(count)*4,length);}for(unsigned j=0;j<count;++j){auto frame=block.u32(table+j*4);assetCheck(frame>=104,"Window frame overlaps header");extent(frame,4,length);auto flip=block.u8(frame+2);assetCheck(flip<=5,"Invalid window texture flip");pane.windowFrames.emplace_back(block.u16(frame),flip);}
            }
            result.panes.push_back(std::move(pane));last=int(result.panes.size()-1);
        }offset+=length;
    }
    assetCheck(foundSize&&offset==data.size()&&parents.empty(),"Incomplete layout resource or hierarchy");for(const auto& pane:result.panes)if(pane.kind=="pic1"||pane.kind=="wnd1"||pane.kind=="txt1"){assetCheck(foundMaterials&&pane.material<result.materialCount,"Unknown pane material");if(pane.text)assetCheck(foundFonts&&pane.text->font<result.fontNames.size(),"Unknown layout text font");for(const auto& frame:pane.windowFrames)assetCheck(frame.first<result.materialCount,"Unknown window frame material");}for(const auto& material:result.materials)for(const auto& map:material.textures)assetCheck(map.texture<result.textureNames.size(),"Unknown layout material texture");return result;
}


struct NativeLayoutAssets{NativeLayoutResource layout;std::vector<TextureImage> textures;};
inline NativeLayoutAssets loadNativeLayoutAssets(Rarc& archive,const std::string& path){
    auto bytes=archive.readFile(path);NativeLayoutAssets result;result.layout=decodeLayoutResource(BinaryView(bytes));
    for(auto name:result.layout.textureNames){std::transform(name.begin(),name.end(),name.begin(),[](unsigned char c){return char(std::tolower(c));});auto data=archive.readFile("/timg/"+name);auto images=decodeTpl(BinaryView(data));assetCheck(images.size()==1,"Unsupported multi-image layout TPL");auto view=BinaryView(data);auto header=view.u32(view.u32(8));auto& image=images.front();auto wrapS=view.u32(header+12),wrapT=view.u32(header+16),minFilter=view.u32(header+20),magFilter=view.u32(header+24);assetCheck(wrapS<=2&&wrapT<=2&&minFilter<=1&&magFilter<=1&&view.u8(header+33)==0&&view.u8(header+34)==0&&view.f32(header+28)==0,"Unsupported layout TPL sampler/LOD");image.wrapS=uint8_t(wrapS);image.wrapT=uint8_t(wrapT);image.minFilter=uint8_t(minFilter);image.magFilter=uint8_t(magFilter);result.textures.push_back(std::move(image));}return result;
}




