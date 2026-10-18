#pragma once
#include "message_text.hpp"
#include "native_message_text_binding.hpp"
#include <Game/System/MessageHolder.hpp>
#include <cstring>
#include <Game/Util/JMapInfo.hpp>
#include <Game/NPC/TalkNodeCtrl.hpp>
class NativeMessageRecords {
    std::vector<uint64_t> infoStorage,dataStorage,flowStorage;
    NativeMessageTextOwners hostText;
public:
    ~NativeMessageRecords(){unbindNativeMessageTextCopies(&hostText);nativeMessageTextBindings().erase(&view);}
    const NativeTaggedMessage& messageText(size_t index)const{assetCheck(index<hostText.size(),"Invalid host message index");return *hostText[index];}
    JMapInfo ids;
    MessageData view;
    void attachIds(const std::vector<uint8_t>& bytes){assetCheck(ids.attachNative(bytes),"Invalid native message ID table");StageTable table{BinaryView(bytes)};auto index=table.find("Index");assetCheck(index>=0,"Missing message index column");for(size_t row=0;row<table.rows;++row){auto value=table.read(row,size_t(index));assetCheck(std::holds_alternative<uint32_t>(value) && std::get<uint32_t>(value)<view.mInfoBlock->mItemCount,"Message ID outside info table");}view.mIDTable=&ids;}
    NativeMessageRecords(const NativeMessageRecords&)=delete;
    NativeMessageRecords& operator=(const NativeMessageRecords&)=delete;
    explicit NativeMessageRecords(const std::vector<uint8_t>& bytes){
        NativeBmgText validated(bytes);BinaryView data(bytes);
        hostText.reserve(validated.size());for(size_t i=0;i<validated.size();++i)hostText.emplace_back(std::make_unique<NativeTaggedMessage>(validated.tokens(i)));
        size_t cursor=32,info=0,text=0,infoBytes=0,textBytes=0;
        for(unsigned i=0;i<data.u32(12);++i){extent(cursor,8,data.u32(8));auto length=data.u32(cursor+4);assetCheck(length>=8,"Invalid message block size");extent(cursor,length,data.u32(8));
            if(data.tag(cursor)=="INF1"){assetCheck(!info,"Duplicate message info");info=cursor;infoBytes=length;}
            if(data.tag(cursor)=="DAT1"){assetCheck(!text,"Duplicate message text");text=cursor;textBytes=length;}
            cursor+=length;if(info && text)break;
        }
        assetCheck(info && text,"Missing message records");auto count=data.u16(info+8),stride=data.u16(info+10);
        assetCheck(stride>=12 && stride%4==0,"Unsupported original message record stride");extent(info+16,size_t(count)*stride,info+infoBytes);
        infoStorage.resize((infoBytes+7)/8);dataStorage.resize((textBytes+7)/8);
        auto nativeInfo=reinterpret_cast<uint8_t*>(infoStorage.data());std::memcpy(nativeInfo,bytes.data()+info,infoBytes);
        auto header=new(nativeInfo) MessageInfoBlock{};header->mMagic=0x494e4631;header->mBlockSize=uint32_t(infoBytes);header->mItemCount=count;header->mItemSize=stride;header->_C=data.u32(info+12);
        for(unsigned i=0;i<count;++i){auto offset=data.u32(info+16+i*stride);assetCheck(!(offset&1) && size_t(offset)+2<=textBytes-8,"Invalid message record text offset");auto camera=data.u16(info+20+i*stride);std::memcpy(nativeInfo+16+i*stride,&offset,4);std::memcpy(nativeInfo+20+i*stride,&camera,2);}
        auto nativeText=reinterpret_cast<uint8_t*>(dataStorage.data());std::memcpy(nativeText,bytes.data()+text,textBytes);
        auto textHeader=new(nativeText) MessageDataBlock{};textHeader->mMagic=0x44415431;textHeader->mBlockSize=uint32_t(textBytes);
        view.mInfoBlock=header;view.mDataBlock=textHeader;
        cursor=32;size_t flow=0,flowBytes=0;
        for(unsigned i=0;i<data.u32(12);++i){extent(cursor,8,bytes.size());auto length=data.u32(cursor+4);assetCheck(length>=8,"Invalid trailing message block");extent(cursor,length,bytes.size());if(data.tag(cursor)=="FLW1"){assetCheck(!flow,"Duplicate message flow");flow=cursor;flowBytes=length;break;}cursor+=length;}
        if(flow){
            extent(flow,16,bytes.size());auto nodes=data.u16(flow+8),branches=data.u16(flow+10);
            extent(flow+16,size_t(nodes)*8+size_t(branches)*2,flow+flowBytes);
            flowStorage.resize((flowBytes+7)/8);auto nativeFlow=reinterpret_cast<uint8_t*>(flowStorage.data());std::memcpy(nativeFlow,bytes.data()+flow,flowBytes);
            auto flowHeader=new(nativeFlow) MessageFlowBlock{};flowHeader->mMagic=0x464c5731;flowHeader->mBlockSize=uint32_t(flowBytes);flowHeader->mNodeCount=nodes;flowHeader->_A=branches;flowHeader->_C=data.u32(flow+12);
            auto nativeNodes=reinterpret_cast<TalkNode*>(flowHeader+1);
            for(unsigned i=0;i<nodes;++i){auto p=flow+16+i*8;auto node=new(nativeNodes+i) TalkNode{};node->mNodeType=data.u8(p);node->mGroupID=data.u8(p+1);node->mIndex=data.u16(p+2);if(node->mNodeType==3)node->mUnknown=data.u32(p+4);else{node->mNextIdx=data.u16(p+4);node->mNextGroup=data.u16(p+6);}}
            auto nativeBranches=reinterpret_cast<uint16_t*>(nativeNodes+nodes);
            for(unsigned i=0;i<branches;++i){auto value=data.u16(flow+16+size_t(nodes)*8+i*2);assetCheck(value==0xffff || value<nodes,"Branch outside message node table");nativeBranches[i]=value;}
            for(unsigned i=0;i<nodes;++i){auto& node=nativeNodes[i];
 if(node.mNodeType==1){assetCheck(node.mIndex<count,"Dialogue message outside info table");assetCheck(node.mNextIdx==0xffff || node.mNextIdx<nodes,"Dialogue successor outside node table");}
 else if(node.mNodeType==2){assetCheck(size_t(node.mNextGroup)+1<branches,"Dialogue branch pair outside branch table");}
 else if(node.mNodeType==3){assetCheck(node.mIndex<branches,"Dialogue event successor outside branch table");}
 else assetCheck(false,"Unsupported dialogue node type");
}
view.mFlowBlock=flowHeader;view._14=nativeBranches;view._18=reinterpret_cast<uint8_t*>(nativeBranches+branches);
        }
        assetCheck(nativeMessageTextBindings().emplace(&view,&hostText).second,"Duplicate host message binding");
    }
};






