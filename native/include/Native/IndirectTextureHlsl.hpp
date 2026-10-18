#pragma once
#include <Native/GxCommandState.hpp>
#include <sstream>

namespace Native {
// GX indirect registers are decoded from the same immutable snapshot as TEV.
inline unsigned indirectStageCount(const GxCommandState& state){
 unsigned count=(state.bp(0)>>16)&7;
 if(count>4)throw std::invalid_argument("Invalid GX indirect stage count");
 return count;
}
inline GxCommandState::TevOrder indirectOrder(const GxCommandState& state,unsigned stage){
 if(stage>=4)throw std::out_of_range("GX indirect source stage");
 unsigned value=state.bp(0x27)>>(stage*6);
 return {value&7,(value>>3)&7,7,true};
}
inline bool indirectUsesSample(const GxCommandState& state,unsigned stage){
 auto indirect=state.tevIndirect(stage);
 return indirect.stage<indirectStageCount(state)&&((indirect.matrix&3)||indirect.alpha);
}
inline bool hasIndirectCoordinates(const GxCommandState& state){
 for(unsigned i=0;i<state.tevStageCount();++i){auto v=state.tevIndirect(i);
  if(v.matrix||v.wrapS||v.wrapT||v.addPrevious)return true;
 }
 return false;
}
inline std::array<bool,8> materialTextureSlots(const GxCommandState& state){
 std::array<bool,8> used{};
 for(unsigned i=0;i<state.tevStageCount();++i){
  auto order=state.tevOrder(i);if(order.enabled)used[order.texture]=true;
  if(indirectUsesSample(state,i))used[indirectOrder(state,state.tevIndirect(i).stage).texture]=true;
 }
 return used;
}
inline std::array<bool,8> materialTexCoordSlots(const GxCommandState& state){
 std::array<bool,8> used{};const bool indirect=hasIndirectCoordinates(state);
 for(unsigned i=0;i<state.tevStageCount();++i){
  auto order=state.tevOrder(i);if(order.enabled||indirect)used[order.coordinate]=true;
  if(indirectUsesSample(state,i))used[indirectOrder(state,state.tevIndirect(i).stage).coordinate]=true;
 }
 return used;
}
struct IndirectMatrix {
 std::array<std::array<int,3>,2> rows{};int shift=0;
};
inline IndirectMatrix indirectMatrix(const GxCommandState& state,unsigned index){
 if(index>=3)throw std::out_of_range("GX indirect matrix index");
 IndirectMatrix result;unsigned scale=0;
 for(unsigned col=0;col<3;++col){
  unsigned value=state.bp(6+index*3+col);
  for(unsigned row=0;row<2;++row){unsigned bits=(value>>(row*11))&2047;result.rows[row][col]=bits&1024?int(bits)-2048:int(bits);}
  scale|=((value>>22)&(col==2?1:3))<<(col*2);
 }
 result.shift=17-int(scale);return result;
}
// Generates texture inputs to the existing integer TEV combiner. Regular
// materials keep their established normalized/projective sampling path.
// Indirect coordinates use GX signed 24-bit values with seven fraction bits.
inline std::string modelTextureSamplesHlsl(const GxCommandState& state){
 std::ostringstream s;s<<"int4 textures[16];";
 const bool indirect=hasIndirectCoordinates(state);
 auto projection=[](unsigned coord){auto c=std::to_string(coord);return "(o.uv["+c+"].z==0.0?o.uv["+c+"].xy:o.uv["+c+"].xy/o.uv["+c+"].z)";};
 if(indirect){
  auto slots=materialTextureSlots(state);auto coords=materialTexCoordSlots(state);
  for(unsigned i=0;i<8;++i){
   if(slots[i])s<<"uint width"<<i<<",height"<<i<<";image"<<i<<".GetDimensions(width"<<i<<",height"<<i<<");float2 dims"<<i<<"=float2(width"<<i<<",height"<<i<<")*128.0;";
   if(coords[i])s<<"int2 fixed"<<i<<"=int2("<<projection(i)<<"*float2("<<((state.bp(0x30+i*2)&65535)+1)*128<<","<<((state.bp(0x31+i*2)&65535)+1)*128<<"));";
  }
  std::array<bool,4> sources{};
  for(unsigned i=0;i<state.tevStageCount();++i)if(indirectUsesSample(state,i))sources[state.tevIndirect(i).stage]=true;
  for(unsigned i=0;i<4;++i)if(sources[i]){
   auto order=indirectOrder(state,i);unsigned scale=state.bp(0x25+i/2)>>((i&1)*8);
   s<<"int2 sourceUv"<<i<<"=fixed"<<order.coordinate<<">>int2("<<(scale&15)<<","<<((scale>>4)&15)<<");";
   s<<"int3 offset"<<i<<"=int3(round(saturate(image"<<order.texture<<".Sample(sampling"<<order.texture<<",float2(sourceUv"<<i<<")/dims"<<order.texture<<").abg)*255));";
  }
  s<<"int2 tevUv=int2(0,0);";
 }
 for(unsigned i=0;i<16;++i){
  auto order=i<state.tevStageCount()?state.tevOrder(i):GxCommandState::TevOrder{};
  if(indirect&&i<state.tevStageCount()){
   auto v=state.tevIndirect(i);
   s<<"{int2 translation=int2(0,0);";
   if((v.matrix&3)&&indirectUsesSample(state,i)){
    static const unsigned shifts[]={0,3,4,5};
    s<<"int3 delta=offset"<<v.stage<<">>"<<shifts[v.format]<<";";
    for(unsigned c=0;c<3;++c)if(v.bias&(1<<c))s<<"delta["<<c<<"]+="<<(v.format?1:-128)<<";";
    auto matrix=indirectMatrix(state,(v.matrix&3)-1);unsigned kind=v.matrix>>2;
    if(kind==0){
     for(unsigned row=0;row<2;++row)s<<"translation["<<row<<"]=(delta.x*"<<matrix.rows[row][0]<<"+delta.y*"<<matrix.rows[row][1]<<"+delta.z*"<<matrix.rows[row][2]<<")>>3;";
    }else if(kind==1||kind==2)s<<"translation=(fixed"<<order.coordinate<<"*delta."<<(kind==1?'x':'y')<<")>>8;";
    else throw std::invalid_argument("Unsupported GX indirect matrix selector");
    if(matrix.shift>=0)s<<"translation>>="<<matrix.shift<<";";else s<<"translation<<="<<-matrix.shift<<";";
   }
   s<<"int2 wrapped=fixed"<<order.coordinate<<";";
   for(unsigned c=0;c<2;++c){unsigned wrap=c?v.wrapT:v.wrapS;
    if(wrap>=6)s<<"wrapped["<<c<<"]=0;";
    else if(wrap)s<<"wrapped["<<c<<"]&="<<((256u>>(wrap-1))*128-1)<<";";
   }
   s<<"tevUv"<<(v.addPrevious?"+=":"=")<<"wrapped+translation;tevUv=(tevUv<<8)>>8;}";
  }
  s<<"textures["<<i<<"]=";
  if(i<state.tevStageCount()&&order.enabled){
   auto slot=std::to_string(order.texture);
   s<<"int4(round(saturate(image"<<slot;
   if(indirect){
    auto v=state.tevIndirect(i);s<<(v.unmodifiedLod?".SampleGrad(":".Sample(")<<"sampling"<<slot<<",float2(tevUv)/dims"<<slot;
    if(v.unmodifiedLod)s<<",ddx(float2(fixed"<<order.coordinate<<")/dims"<<slot<<"),ddy(float2(fixed"<<order.coordinate<<")/dims"<<slot<<")";
   }else s<<".Sample(sampling"<<slot<<","<<projection(order.coordinate);
   s<<"))*255));";
  }else s<<"int4(255,255,255,255);";
 }
 return s.str();
}
}
