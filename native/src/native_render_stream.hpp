#pragma once
#include "native_shape_vertices.hpp"
struct NativeRenderOperation {
 enum class Kind {Draw,Copy,Token,Finish};
 Kind kind;Native::GxCommandState state;
 std::vector<NativeShapeVertex> vertices;std::vector<std::array<u32,3>> triangles;
 Native::GxCommandState::CopyOperation copy{};u16 token=0;
};
class NativeRenderStream {
 Native::GxCommandState state_;std::vector<NativeRenderOperation> operations_;
 static unsigned read16(const u8* bytes){return (unsigned(bytes[0])<<8)|bytes[1];}
 static size_t vertexSize(const Native::GxCommandState& state,unsigned vat){
  size_t size=0;u32 low=state.cp(0x50),high=state.cp(0x60);
  for(unsigned i=0;i<9;++i)size+=(low>>i)&1;
  auto position=state.positionFormat(vat);if(!position.descriptor)throw std::invalid_argument("Missing GX position descriptor");
  size+=position.descriptor==1?position.components*(position.type<2?1:position.type<4?2:4):position.descriptor==2?1:2;
  for(unsigned attr=10;attr<=20;++attr){
   unsigned descriptor=attr<=12?(low>>(11+2*(attr-10)))&3:(high>>(2*(attr-13)))&3;
   if(!descriptor)continue;
   if(descriptor>=2){size+=descriptor==2?1:2;continue;}
   if(attr==10)size+=state.directNormalSize(vat);
   else if(attr<=12)size+=state.directColorSize(vat,attr-11);
   else size+=state.directTexCoordSize(vat,attr-13);
  }return size;
 }
 static void parse(Native::GxCommandState& state,std::vector<NativeRenderOperation>& output,const u8* data,size_t size,const Native::GpuResources& resources,unsigned depth,size_t& budget){
  if(depth>32||size>budget)throw std::length_error("GX render stream execution limit");budget-=size;
  if(!data&&size)throw std::invalid_argument("Null GX render stream");
  size_t p=0;
  auto require=[&](size_t length){if(length>size-p)throw std::length_error("Truncated GX render command");};
  while(p<size){
   require(1);unsigned op=data[p];if(op==0){++p;continue;}
   if(output.size()>=65536)throw std::length_error("GX render operation limit");
   if(op==0x40){
    require(9);u32 address=be32(data+p+1),length=be32(data+p+5);p+=9;
    parse(state,output,resources.resolve(address,length),length,resources,depth+1,budget);continue;
   }
   unsigned primitive=op&0xf8;
   if(primitive==0x80||primitive==0x90||primitive==0x98||primitive==0xa0){
    require(3);size_t count=read16(data+p+1),stride=vertexSize(state,op&7);
    if(count>(size-p-3)/stride)throw std::length_error("Truncated GX render primitive");
    size_t length=3+count*stride;NativeRenderOperation draw{NativeRenderOperation::Kind::Draw,state};
    draw.vertices=decodeShapePositions(state,data+p,length,resources,&draw.triangles);
    output.push_back(std::move(draw));p+=length;continue;
   }
   size_t length;
   if(op==0x61||op==0x20||op==0x28||op==0x30||op==0x38)length=5;
   else if(op==8)length=6;
   else if(op==0x48)length=1;
   else if(op==0x10){require(5);length=5+(size_t(read16(data+p+1))+1)*4;}
   else throw std::invalid_argument("Unsupported GX render command");
   require(length);state.consume(data+p,length,&resources);p+=length;
   for(const auto& copy:state.pendingCopies()){NativeRenderOperation event{NativeRenderOperation::Kind::Copy,state};event.copy=copy;output.push_back(std::move(event));}
   for(const auto& sync:state.pendingSyncEvents()){NativeRenderOperation event{sync.kind==Native::GxCommandState::SyncEvent::Kind::Token?NativeRenderOperation::Kind::Token:NativeRenderOperation::Kind::Finish,state};event.token=sync.token;output.push_back(std::move(event));}
   state.clearPendingCopies();state.clearPendingSyncEvents();
  }
 }
public:
 void decode(const u8* data,size_t size,const Native::GpuResources& resources){
  auto next=state_;std::vector<NativeRenderOperation> output;size_t budget=64*1024*1024;
  parse(next,output,data,size,resources,0,budget);state_=std::move(next);operations_=std::move(output);
 }
 const auto& operations()const{return operations_;}
 const auto& state()const{return state_;}
};

