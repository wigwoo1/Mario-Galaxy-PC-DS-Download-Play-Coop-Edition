#pragma once
#include "stage_data.hpp"
#include <optional>
// Bounded INF1/DAT1 reader. Tag payloads remain Wii bytes until typed native conversion.
class NativeBmgText {
    std::vector<uint8_t> bytes;std::vector<uint32_t> offsets;size_t start=0,end=0;
public:
    explicit NativeBmgText(std::vector<uint8_t> resource):bytes(std::move(resource)){
        BinaryView view(bytes);assetCheck(view.size()>=32&&view.tag(0)=="MESG"&&view.tag(4)=="bmg1"&&view.u8(16)==2,"Unsupported BMG header/encoding");size_t core=view.u32(8);assetCheck(core>=32&&core<=view.size(),"Invalid BMG core size");auto count=view.u32(12);assetCheck(count>0&&count<=1024,"Invalid BMG block count");size_t cursor=32,info=0,infoLength=0;
        for(unsigned i=0;i<count;++i){extent(cursor,8,core);auto length=view.u32(cursor+4);assetCheck(length>=8,"Invalid BMG block size");extent(cursor,length,core);auto tag=view.tag(cursor);if(tag=="INF1"){assetCheck(!info&&length>=16,"Invalid duplicate BMG info");info=cursor;infoLength=length;}else if(tag=="DAT1"){assetCheck(!start,"Duplicate BMG text block");start=cursor+8;end=cursor+length;}cursor+=length;if(info&&start)break;}
        assetCheck(info&&start,"Missing BMG text/info blocks");auto items=view.u16(info+8),stride=view.u16(info+10);assetCheck(stride>=4,"Invalid BMG info stride");extent(info+16,uint64_t(items)*stride,info+infoLength);for(unsigned i=0;i<items;++i){auto offset=view.u32(info+16+i*stride);assetCheck(!(offset&1)&&uint64_t(offset)+2<=end-start,"Invalid BMG text offset");offsets.push_back(offset);}
    }
    struct Token {
        uint16_t code=0; std::vector<uint8_t> tag;
        void validateTag()const{assetCheck(code==0x1a && tag.size()>=4 && tag.size()<=253 && tag[0]==tag.size()+2 && !(tag[0]&1),"Invalid native message tag");}
        uint8_t group()const{validateTag();return tag[1];}
        uint16_t id()const{validateTag();return uint16_t(uint16_t(tag[2])<<8 | tag[3]);}
        size_t parameterBytes()const{validateTag();return tag.size()-4;}
        uint8_t param8(size_t index)const{assetCheck(index<parameterBytes(),"Tag byte parameter outside payload");return tag[4+index];}
        uint16_t param16(size_t index)const{auto size=parameterBytes();assetCheck(size>=2 && index<=(size-2)/2,"Tag halfword parameter outside payload");auto p=4+index*2;return uint16_t(uint16_t(tag[p])<<8 | tag[p+1]);}
        uint32_t param32(size_t index)const{auto size=parameterBytes();assetCheck(size>=4 && index<=(size-4)/4,"Tag word parameter outside payload");auto p=4+index*4;return uint32_t(tag[p])<<24 | uint32_t(tag[p+1])<<16 | uint32_t(tag[p+2])<<8 | tag[p+3];}
    };
    std::vector<Token> tokens(size_t index)const{
        assetCheck(index<offsets.size(),"BMG message index outside table");
        BinaryView view(bytes);std::vector<Token> result;
        size_t cursor=start+offsets[index];
        while(cursor+2<=end){
            auto unit=view.u16(cursor);if(!unit)return result;
            Token token;token.code=unit;
            if(unit==0x1a){
                extent(cursor,6,end);auto length=view.u8(cursor+2);
                assetCheck(length>=6 && !(length&1),"Invalid BMG inline tag size");
                extent(cursor,length,end);
                token.tag.assign(bytes.begin()+cursor+2,bytes.begin()+cursor+length);
                cursor+=length;
            }else cursor+=2;
            result.push_back(std::move(token));
        }
        throw std::runtime_error("Unterminated BMG tagged message");
    }
    size_t size()const{return offsets.size();}
    std::wstring plainHostText(size_t index)const{
        static_assert(sizeof(wchar_t)==2,"Original native message runtime requires UTF16 wchar_t");
        auto text=plainText(index);return std::wstring(text.begin(),text.end());
    }
    std::u16string plainText(size_t index)const{
        assetCheck(index<offsets.size(),"BMG message index outside table");BinaryView view(bytes);std::u16string result;for(size_t cursor=start+offsets[index];cursor+2<=end;cursor+=2){auto unit=view.u16(cursor);if(!unit)return result;assetCheck(unit!=0x1A,"Inline BMG message tags require a native tag processor");result.push_back(char16_t(unit));}throw std::runtime_error("Unterminated BMG UTF-16 message");
    }
};
inline std::optional<uint32_t> nativeMessageIndex(const std::vector<std::pair<std::string,uint32_t>>& ids,const std::string& id){size_t low=0,high=ids.size();while(low<high){size_t mid=(low+high)/2;int comparison=ids[mid].first.compare(id);if(!comparison)return ids[mid].second;if(comparison<0)low=mid+1;else high=mid;}return std::nullopt;}
class NativeMessageCatalog {
    NativeBmgText messages;std::vector<std::pair<std::string,uint32_t>> ids;
public:
    explicit NativeMessageCatalog(Rarc& archive):messages(archive.readFile("/message.bmg")){
        auto bytes=archive.readFile("/messageid.tbl");StageTable table{BinaryView(bytes)};int name=table.find("MessageId"),index=table.find("Index");assetCheck(name>=0&&index>=0,"Missing message ID table columns");for(size_t row=0;row<table.rows;++row){auto key=table.read(row,size_t(name)),value=table.read(row,size_t(index));assetCheck(std::holds_alternative<std::string>(key)&&std::holds_alternative<uint32_t>(value),"Invalid message ID table column types");auto id=std::get<std::string>(key);auto number=std::get<uint32_t>(value);assetCheck(number<messages.size(),"Invalid message ID index");ids.emplace_back(std::move(id),number);}
    }
    size_t size()const{return ids.size();}
    std::u16string plainText(const std::string& id)const{auto found=nativeMessageIndex(ids,id);assetCheck(found.has_value(),"Missing localized message ID");return messages.plainText(*found);}
};
inline void bmgTextTests(){
    std::vector<std::pair<std::string,uint32_t>> ids{{"",0},{"A",1},{"A",2},{"B",3}};assetCheck(nativeMessageIndex(ids,"A")==std::optional<uint32_t>{2}&&nativeMessageIndex(ids,"")==std::optional<uint32_t>{0}&&!nativeMessageIndex(ids,"Missing"),"Original message binary lookup/sentinel/duplicate behavior mismatch");
    std::vector<uint8_t> bytes(96);auto word=[&](size_t p,uint32_t v){for(unsigned i=0;i<4;++i)bytes[p+i]=uint8_t(v>>(24-i*8));};auto half=[&](size_t p,uint16_t v){bytes[p]=uint8_t(v>>8);bytes[p+1]=uint8_t(v);};std::memcpy(bytes.data(),"MESGbmg1",8);word(8,96);word(12,2);bytes[16]=2;std::memcpy(bytes.data()+32,"INF1",4);word(36,32);half(40,1);half(42,12);word(48,2);std::memcpy(bytes.data()+64,"DAT1",4);word(68,32);half(74,'O');half(76,'K');assetCheck(NativeBmgText(bytes).plainText(0)==u"OK","BMG text offset/UTF-16 mismatch");
    auto rejects=[&](){bool rejected=false;try{NativeBmgText(bytes).plainText(0);}catch(const std::exception&){rejected=true;}assetCheck(rejected,"Malformed/unsupported BMG text accepted");};half(74,0x1A);rejects();half(74,'O');word(48,23);rejects();word(48,2);word(68,40);rejects();word(68,32);for(size_t p=78;p<96;p+=2)half(p,'X');rejects();
}




