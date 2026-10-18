#pragma once
#include "native_render_stream.hpp"
#include <Native/IndirectTextureHlsl.hpp>
#include <fstream>
#include <filesystem>
#include <cstdlib>
#include <type_traits>

// Diagnostic-only projection replay. No GPU resources or original game state
// are changed. Register commands omit copy/palette/sync triggers.
inline void captureNativeProjectionFailure(const NativeRenderOperation& op,bool lighting,const char* error){
 const auto* destination=std::getenv("GALAXY_PROJECTION_FAILURE_DIR");if(!destination||!*destination)return;
 static_assert(std::is_trivially_copyable<NativeShapeVertex>::value,"Replay vertex format");
 auto dir=std::filesystem::u8path(destination);std::filesystem::create_directories(dir);
 std::vector<u8> commands;
 auto word=[&](u32 value){for(int shift=24;shift>=0;shift-=8)commands.push_back(u8(value>>shift));};
 for(unsigned reg=0;reg<256;++reg){commands.push_back(8);commands.push_back(u8(reg));word(op.state.cp(reg));}
 auto bp=[&](unsigned reg){commands.push_back(0x61);word((reg<<24)|op.state.bp(reg));};
 bp(0);for(unsigned reg=0x10;reg<0x40;++reg)bp(reg);for(unsigned reg=0xc0;reg<0xfe;++reg)bp(reg);
 for(unsigned reg=0;reg<0x1100;++reg)if(op.state.xfLoaded(reg)){commands.push_back(0x10);word(reg);word(op.state.xf(reg));}
 std::ofstream binary(dir/"projection-replay.bin",std::ios::binary);
 binary.write("GXMRF001",8);
 auto size=[&](u32 value){binary.write(reinterpret_cast<const char*>(&value),4);};
 size(sizeof(NativeShapeVertex));size(u32(op.vertices.size()));size(u32(op.triangles.size()));size(u32(lighting));size(u32(commands.size()));
 binary.write(reinterpret_cast<const char*>(commands.data()),commands.size());
 binary.write(reinterpret_cast<const char*>(op.vertices.data()),op.vertices.size()*sizeof(NativeShapeVertex));
 binary.write(reinterpret_cast<const char*>(op.triangles.data()),op.triangles.size()*sizeof(std::array<u32,3>));
 std::ofstream text(dir/"projection-failure.txt");text<<error<<"\nvertices="<<op.vertices.size()<<" triangles="<<op.triangles.size()<<" lighting="<<lighting<<"\n";
 text<<std::hex<<"genmode="<<op.state.bp(0)<<" texgens="<<op.state.xf(0x103f)<<" vcdLow="<<op.state.cp(0x50)<<" vcdHigh="<<op.state.cp(0x60)<<"\n";
 auto used=Native::materialTexCoordSlots(op.state);
 for(unsigned i=0;i<8;++i)text<<"texgen "<<i<<" used="<<used[i]<<" descriptor="<<op.state.xf(0x1040+i)<<" dual="<<op.state.xf(0x1050+i)<<"\n";
 for(unsigned i=0;i<op.state.tevStageCount();++i){auto order=op.state.tevOrder(i);text<<"tev "<<i<<" enabled="<<order.enabled<<" coordinate="<<order.coordinate<<" texture="<<order.texture<<" indirect="<<op.state.bp(0x10+i)<<"\n";}
 if(!op.vertices.empty()){const auto& v=op.vertices[0];text<<"vertex UV presence=";for(bool present:v.hasTexCoords)text<<present;text<<" legacy0="<<v.hasTexCoord0<<"\n";}
}
