#include "native_tagged_message.hpp"
#include <Game/Screen/MessageTagSkipTagProcessor.hpp>
#include "native_message_arguments.hpp"
#include "native_disc_archive_loader.hpp"
#include "native_message_records.hpp"
#include "native_disc_message_backend.hpp"
#include <Game/NPC/TalkMessageInfo.hpp>
#include <cassert>
#include <cstdio>
int main(int argc,char**argv){assert(argc==2);NativeDiscArchiveLoader disc(std::filesystem::u8path(argv[1]));auto archive=disc.mount("/UsEnglish/MessageData/Message.arc");auto resource=archive->get("/message.bmg");assert(resource);
std::vector<uint8_t> bytes(resource,resource+archive->sizeOf(resource));BinaryView data(bytes);size_t cursor=32,info=0,text=0,flowOffset=0;for(unsigned i=0;i<data.u32(12);++i){if(data.tag(cursor)=="INF1")info=cursor;if(data.tag(cursor)=="DAT1")text=cursor;if(data.tag(cursor)=="FLW1")flowOffset=cursor;cursor+=data.u32(cursor+4);}assert(info && text);
NativeBmgText::Token fixture;fixture.code=0x1a;fixture.tag={14,7,0x12,0x34,0x01,0x23,0x45,0x67,0x89,0xab,0xcd,0xef};
assert(fixture.group()==7 && fixture.id()==0x1234 && fixture.parameterBytes()==8);
assert(fixture.param8(7)==0xef && fixture.param16(1)==0x4567 && fixture.param32(0)==0x01234567 && fixture.param32(1)==0x89abcdef);
for(unsigned width : {1u,2u,4u})for(size_t index : {size_t(8),size_t(-1)}){bool rejected=false;try{if(width==1)fixture.param8(index);else if(width==2)fixture.param16(index);else fixture.param32(index);}catch(const std::runtime_error&){rejected=true;}assert(rejected);}
std::puts("PASS typed tag endian decoding and out-of-range parameter rejection");
{NativeTaggedMessage owner({fixture});const wchar_t* payload=owner.data()+1;MessageEditorMessageTag tag(payload);assert(tag.getTag()==0x1234&&tag.getGroup()==7);
{NativeMessageTagScope scope(owner);assert(tag.getGroup()==7 && tag.getTag()==0x1234 && tag.isGroupTagId(7,0x1234));assert(tag.getTagLength()==12 && tag.getSkipLength()==6 && tag.getParamLength()==8);assert(tag.getParam8(7)==0xef && tag.getParam16(1)==0x4567 && tag.getParam32(1)==0x89abcdef);for(int bad:{-1,8}){bool rejected=false;try{tag.getParam32(bad);}catch(const std::exception&){rejected=true;}assert(rejected);}NativeTaggedMessage nested({fixture});{NativeMessageTagScope inner(nested);bool wrongOwner=false;try{tag.getTag();}catch(const std::exception&){wrongOwner=true;}assert(wrongOwner);assert(MessageEditorMessageTag(nested.data()+1).getTag()==0x1234);}assert(tag.getTag()==0x1234);}
assert(nativeTagOwner==nullptr);}
std::puts("PASS original tag numeric reader: native endian values, bounds and nested owner scopes");
{NativeMessageTextOwners owners;owners.emplace_back(std::make_unique<NativeTaggedMessage>(std::vector<NativeBmgText::Token>{fixture}));auto binding=nativeMessageTextBindings().emplace(nullptr,&owners);assert(binding.second);std::vector<wchar_t> copied(owners[0]->data(),owners[0]->data()+owners[0]->size()+1);bindNativeMessageTextCopy(copied.data(),owners[0]->size()+1,owners[0]->data());MessageEditorMessageTag copiedTag(copied.data()+1);assert(copiedTag.getGroup()==7&&copiedTag.getTag()==0x1234&&copiedTag.getParam32(1)==0x89abcdef);unbindNativeMessageTextCopies(copied.data(),copied.size());nativeMessageTextBindings().erase(nullptr);assert(nativeMessageTextCopies().empty());}
std::puts("PASS copied text-box message tags resolve to their retained BMG owner");
{NativeBmgText::Token textTag;textTag.code=0x1a;textTag.tag={14,5,0,0,0x00,0x41,0x00,0xe9,0xd8,0x3d,0xde,0x00};NativeTaggedMessage owner({textTag});NativeMessageTagScope scope(owner);const wchar_t* p=owner.data()+1;const wchar_t* converted=nativeMessageTagText(p,0,4);assert(converted[0]==L'A' && converted[1]==0xe9 && converted[2]==0xd83d && converted[3]==0xde00 && converted[4]==0);assert(nativeMessageTagText(p,0,4)==converted);assert(nativeMessageTagText(p,2,1)[0]==0xe9 && converted[0]==L'A');for(size_t offset:{size_t(9),size_t(-1)}){bool rejected=false;try{nativeMessageTagText(p,offset,1);}catch(const std::exception&){rejected=true;}assert(rejected);}bool overflow=false;try{nativeMessageTagText(p,0,size_t(-1));}catch(const std::exception&){overflow=true;}assert(overflow);}
{NativeBmgText::Token ruby;ruby.code=0x1a;ruby.tag={12,0,0,0,2,0,0x41,0,0xe9,0};NativeTaggedMessage owner({ruby});NativeMessageTagScope scope(owner);auto text=nativeMessageTagText(owner.data()+1,1,2);assert(text[0]==L'A' && text[1]==0xe9 && text[2]==0);}
std::puts("PASS owned inline UTF16 tag fields: endian conversion, odd ruby offset, stable terminator and bounds");
{NativeBmgText::Token a;a.code=L'A';NativeBmgText::Token b;b.code=0xe9;NativeTaggedMessage owner({fixture,a,b});NativeMessageTagScope scope(owner);auto p=owner.data()+1;auto base=nativeMessageTagBase(p,2);assert(base==owner.afterTag(p) && base[0]==L'A' && base[1]==0xe9);for(size_t count:{size_t(3),size_t(-1)}){bool rejected=false;try{nativeMessageTagBase(p,count);}catch(const std::exception&){rejected=true;}assert(rejected);}}
{NativeTaggedMessage owner({fixture,fixture});NativeMessageTagScope scope(owner);bool rejected=false;try{nativeMessageTagBase(owner.data()+1,1);}catch(const std::exception&){rejected=true;}assert(rejected);}
std::puts("PASS ruby base host span: exact following text, bounds and embedded-tag rejection");
NativeMessageArguments arguments;auto argumentTag=fixture;argumentTag.tag[1]=7;argumentTag.tag[8]=0;argumentTag.tag[9]=0;argumentTag.tag[10]=0;argumentTag.tag[11]=3;
auto originalBytes=argumentTag.tag;{std::wstring temporary=L"Luigi";arguments.setString(3,temporary);temporary.assign(L"discarded");}
assert(arguments.string(argumentTag) && *arguments.string(argumentTag)==L"Luigi");
arguments.setString(3,L"Mario");assert(*arguments.string(argumentTag)==L"Mario");
argumentTag.tag[1]=6;arguments.setNumber(3,-2147483647);assert(arguments.number(argumentTag)==-2147483647);
argumentTag.tag[1]=7;assert(argumentTag.tag==originalBytes);arguments.clear();assert(!arguments.string(argumentTag));argumentTag.tag[1]=6;assert(!arguments.number(argumentTag));
std::puts("PASS owned native string/number arguments, replacement, clearing and unchanged tag bytes");
NativeBmgText tagged(bytes);size_t totalTokens=0,totalTags=0;
for(size_t i=0;i<tagged.size();++i){auto tokens=tagged.tokens(i);for(auto& t:tokens){++totalTokens;if(t.code==0x1a){++totalTags;assert(t.tag.size()>=4 && t.tag[0]==t.tag.size()+2);assert(t.group()==t.tag[1]);assert(t.id()==(uint16_t(t.tag[2])<<8 | t.tag[3]));for(size_t p=0;p<t.parameterBytes();++p)assert(t.param8(p)==t.tag[4+p]);}}}
size_t plainMessages=0;
for(size_t i=0;i<tagged.size();++i){auto tokens=tagged.tokens(i);bool hasTags=false;for(auto& token:tokens)hasTags|=token.code==0x1a;if(hasTags){bool rejected=false;try{tagged.plainHostText(i);}catch(const std::exception&){rejected=true;}assert(rejected);}else{auto host=tagged.plainHostText(i);assert(host.size()==tokens.size());for(size_t j=0;j<host.size();++j)assert(uint16_t(host[j])==tokens[j].code);++plainMessages;}}
std::printf("PASS %zu bounded host UTF16 plain messages; tagged messages rejected\n",plainMessages);
size_t hostTags=0;
for(size_t i=0;i<tagged.size();++i){auto tokens=tagged.tokens(i);NativeTaggedMessage host(tokens);NativeMessageTagScope scope(host);size_t offset=0,count=0;for(auto& token:tokens){assert(uint16_t(host.data()[offset])==token.code);++offset;if(token.code==0x1a){auto& metadata=host.tagAt(offset);MessageEditorMessageTag original(host.data()+offset);assert(original.getTag()==token.id() && original.getGroup()==token.group() && original.getSkipLength()==token.tag.size()/2);for(size_t k=0;k<token.parameterBytes();++k)assert(original.getParam8(int(k))==token.param8(k));for(size_t k=0;k<token.parameterBytes()/2;++k)assert(original.getParam16(int(k))==token.param16(k));for(size_t k=0;k<token.parameterBytes()/4;++k)assert(original.getParam32(int(k))==token.param32(k));assert(&host.tagAt(host.data()+offset)==&metadata);assert(host.afterTag(host.data()+offset)==host.data()+offset+token.tag.size()/2);bool interiorRejected=false;try{host.tagAt(host.data()+offset+1);}catch(const std::exception&){interiorRejected=true;}assert(interiorRejected);assert(metadata.code==token.code && metadata.tag==token.tag);assert(!std::memcmp(host.data()+offset,token.tag.data(),token.tag.size()));offset+=token.tag.size()/2;++count;}}assert(offset==host.size() && host.data()[offset]==0 && count==host.tagCount());hostTags+=count;bool nullRejected=false;try{host.tagAt(static_cast<const wchar_t*>(nullptr));}catch(const std::exception&){nullRejected=true;}assert(nullRejected);wchar_t foreign[2]={0};bool foreignRejected=false;try{host.afterTag(foreign);}catch(const std::exception&){foreignRejected=true;}assert(foreignRejected);}
std::printf("PASS owned host tagged messages: all %zu tag payloads/positions preserved\n",hostTags);
size_t taggedIndex=0,tagPosition=0;
for(size_t i=0;i<tagged.size() && !tagPosition;++i){size_t p=text+8+data.u32(info+16+i*data.u16(info+10));while(data.u16(p)){if(data.u16(p)==0x1a){taggedIndex=i;tagPosition=p;break;}p+=2;}}
assert(tagPosition);
for(uint8_t size : {uint8_t(0),uint8_t(4),uint8_t(7)}){auto bad=bytes;bad[tagPosition+2]=size;bool rejected=false;try{NativeBmgText invalid(bad);invalid.tokens(taggedIndex);}catch(const std::runtime_error&){rejected=true;}assert(rejected);}
std::puts("PASS malformed inline tag size rejection");
std::printf("PASS bounded actual BMG token parsing: %zu tokens, %zu inline tags\n",totalTokens,totalTags);
const MessageData* releasedView=nullptr;{auto source=bytes;NativeMessageRecords lifetime(source);releasedView=&lifetime.view;source.clear();source.shrink_to_fit();TalkMessageInfo returned;assert(lifetime.view.getMessage(&returned,0,0));auto expected=tagged.tokens(0);auto& textOwner=lifetime.messageText(0);NativeMessageTagScope scope(textOwner);size_t offset=0;for(const auto& token:expected){assert(uint16_t(reinterpret_cast<const wchar_t*>(returned._0)[offset++])==token.code);if(token.code==0x1a){MessageEditorMessageTag original(textOwner.data()+offset);assert(original.getTag()==token.id());offset+=original.getSkipLength();}}assert(offset==textOwner.size());assert(!lifetime.view.getMessage(nullptr,0,0));auto prior=returned._0;assert(!lifetime.view.getMessage(&returned,0,lifetime.view.mInfoBlock->mItemCount));assert(returned._0==prior);}
assert(nativeMessageTextBindings().find(releasedView)==nativeMessageTextBindings().end());
std::puts("PASS original getter host-text lifetime after source release and binding cleanup");
NativeMessageRecords records(bytes);for(unsigned i=0;i<records.view.mInfoBlock->mItemCount;++i){TalkMessageInfo message;assert(records.view.getMessage(&message,0,uint16_t(i)));auto item=info+16+i*data.u16(info+10);assert(message.mCameraSetID==data.u16(item+4));assert(message.mCameraType==data.u8(item+7));assert(message.mTalkType==data.u8(item+8));assert(message.mBalloonType==data.u8(item+9));assert(message._0==nativeMessageText(records.view,i));assert(reinterpret_cast<const wchar_t*>(message._0)==records.messageText(i).data());}
auto tableBytes=archive->get("/messageid.tbl");assert(tableBytes);std::vector<uint8_t> table(tableBytes,tableBytes+archive->sizeOf(tableBytes));records.attachIds(table);
StageTable reference{BinaryView(table)};auto nameField=reference.find("MessageId"),indexField=reference.find("Index");assert(nameField>=0 && indexField>=0);
for(size_t row=0;row<reference.rows;++row){auto name=std::get<std::string>(reference.read(row,size_t(nameField)));auto index=std::get<uint32_t>(reference.read(row,size_t(indexField)));assert(records.view.findMessageIndex(name.c_str())==int(index));TalkNode* expectedEntry=nullptr;for(unsigned n=0;n<records.view.mFlowBlock->mNodeCount;++n){auto p=flowOffset+16+n*8;if(data.u8(p)==1 && data.u16(p+2)==index){expectedEntry=records.view.getNode(n);break;}}assert(records.view.findNode(name.c_str())==expectedEntry);TalkMessageInfo message;assert(records.view.getMessageDirect(&message,name.c_str()));assert(message._0==nativeMessageText(records.view,index));}
assert(!records.view.findNode("__missing_message__"));
auto reject=[&](size_t p,uint16_t value){auto bad=bytes;bad[p]=uint8_t(value>>8);bad[p+1]=uint8_t(value);bool rejected=false;try{NativeMessageRecords invalid(bad);}catch(const std::runtime_error&){rejected=true;}assert(rejected);};
bool checkedMessage=false,checkedBranch=false,checkedEvent=false;
for(unsigned i=0;i<records.view.mFlowBlock->mNodeCount;++i){auto p=flowOffset+16+i*8;auto type=data.u8(p);
 if(type==1 && !checkedMessage){reject(p+2,records.view.mInfoBlock->mItemCount);reject(p+4,records.view.mFlowBlock->mNodeCount);checkedMessage=true;}
 if(type==2 && !checkedBranch){reject(p+6,records.view.mFlowBlock->_A-1);checkedBranch=true;}
 if(type==3 && !checkedEvent){reject(p+2,records.view.mFlowBlock->_A);checkedEvent=true;}
}
assert(checkedMessage && checkedBranch && checkedEvent);
std::printf("PASS message-ID entry lookup and malformed dialogue-edge rejection\n");
TalkMessageInfo missing;assert(!records.view.getMessageDirect(&missing,"__missing_message__"));
assert(records.view.mFlowBlock);auto flow=records.view.mFlowBlock;
for(unsigned i=0;i<flow->mNodeCount;++i){auto node=records.view.getNode(i);auto p=flowOffset+16+i*8;assert(node->mNodeType==data.u8(p) && node->mGroupID==data.u8(p+1) && node->mIndex==data.u16(p+2));if(node->mNodeType==3)assert(node->mUnknown==data.u32(p+4));else assert(node->mNextIdx==data.u16(p+4) && node->mNextGroup==data.u16(p+6));}
for(unsigned i=0;i<flow->_A;++i){assert(records.view._14[i]==data.u16(flowOffset+16+size_t(flow->mNodeCount)*8+i*2));bool valid=records.view.isValidBranchNode(i);assert(valid==(records.view._14[i]!=0xffff));if(valid)assert(records.view.getBranchNode(i)==records.view.getNode(records.view._14[i]));}
std::printf("PASS %u native dialogue nodes and %u branch entries\n",flow->mNodeCount,flow->_A);
MessageHolder holder;NativeDiscMessageBackend backend(disc);nativeBindGameMessageBackend(&backend);holder.initSystemData();holder.initGameData();holder.initSceneData();assert(holder.mSystemMessageData && holder.mSceneMessageData==holder.mGameMessageData);assert(MessageSystem::getSceneMessageData()==holder.mGameMessageData);
TalkNodeCtrl ctrl;auto scene=holder.mSceneMessageData;
for(unsigned i=0;i<scene->mFlowBlock->mNodeCount;++i){
 ctrl.mCurrentNode=scene->getNode(i);auto node=ctrl.mCurrentNode;TalkNode* expected=nullptr;
 if(node->mNodeType==1 && node->mNextIdx!=0xffff){assert(node->mNextIdx<scene->mFlowBlock->mNodeCount);expected=scene->getNode(node->mNextIdx);}
 if(node->mNodeType==3){assert(node->mIndex<scene->mFlowBlock->_A);if(scene->isValidBranchNode(node->mIndex))expected=scene->getBranchNode(node->mIndex);}
 assert(ctrl.getNextNode()==expected);assert(ctrl.isExistNextNode()==bool(expected));assert(ctrl.isNextNodeMessage()==bool(expected && expected->mNodeType==1));
 assert(ctrl.getCurrentNodeMessage()==(node->mNodeType==1?node:nullptr));assert(ctrl.getCurrentNodeBranch()==(node->mNodeType==2?node:nullptr));assert(ctrl.isCurrentNodeEvent()==(node->mNodeType==3));
 assert(ctrl.getNextNodeBranch()==(expected && expected->mNodeType==2?expected:nullptr));assert(ctrl.getNextNodeEvent()==(expected && expected->mNodeType==3?expected:nullptr));
}
ctrl.mCurrentNode=nullptr;assert(!ctrl.getNextNode() && !ctrl.isExistNextNode());
std::printf("PASS original TalkNodeCtrl navigation across %u scene nodes\n",scene->mFlowBlock->mNodeCount);
holder.destroySceneData();bool unavailable=false;try{MessageSystem::getSceneMessageData();}catch(const std::runtime_error&){unavailable=true;}assert(unavailable);assert(!holder.mSceneMessageData);holder.initSceneData();assert(backend.release(holder));assert(!holder.mSystemMessageData && !holder.mGameMessageData && !holder.mSceneMessageData);
std::printf("PASS original message getter against %u native-owned INF1 records from WBFS\n",records.view.mInfoBlock->mItemCount);
}





















