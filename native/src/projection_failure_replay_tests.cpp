#include "native_model_projection.hpp"
#include <fstream>
#include <iostream>
#include <iterator>
int main(int argc,char** argv){
 try{
  if(argc!=2)throw std::runtime_error("Supply the saved projection-replay.bin path");
  std::ifstream input(std::filesystem::u8path(argv[1]),std::ios::binary);if(!input)throw std::runtime_error("Replay file could not be opened");
  auto read=[&](void* data,size_t count){if(!input.read(static_cast<char*>(data),count))throw std::runtime_error("Truncated projection replay");};
  char magic[8];read(magic,8);if(std::memcmp(magic,"GXMRF001",8))throw std::runtime_error("Unknown projection replay format");
  auto word=[&](){u32 value;read(&value,4);return value;};
  const auto stride=word(),vertexCount=word(),triangleCount=word(),lighting=word(),commandCount=word();
  if(stride!=sizeof(NativeShapeVertex)||vertexCount>100000||triangleCount>100000||commandCount>100000)throw std::runtime_error("Incompatible or oversized projection replay");
  std::vector<u8> commands(commandCount);read(commands.data(),commands.size());
  std::vector<NativeShapeVertex> vertices(vertexCount);read(vertices.data(),vertices.size()*sizeof(NativeShapeVertex));
  std::vector<std::array<u32,3>> triangles(triangleCount);read(triangles.data(),triangles.size()*sizeof(std::array<u32,3>));
  if(input.peek()!=std::char_traits<char>::eof())throw std::runtime_error("Unexpected projection replay suffix");
  Native::GxCommandState state;state.consume(commands.data(),commands.size());
  std::cout<<"Replay original draw: vertices="<<vertexCount<<" triangles="<<triangleCount<<" lighting="<<lighting<<"\n";
  auto projected=projectModelTriangles(vertices,triangles,state,{1,1,1,1},lighting!=0);
  if(projected.size()!=triangles.size()*3)throw std::runtime_error("Replay triangle count changed");
  for(const auto& vertex:projected){for(float x:vertex.position)if(!std::isfinite(x))throw std::runtime_error("Nonfinite replay position");for(float x:vertex.uv)if(!std::isfinite(x))throw std::runtime_error("Nonfinite replay UV");for(const auto& uv:vertex.extraUv)for(float x:uv)if(!std::isfinite(x))throw std::runtime_error("Nonfinite replay extra UV");}
  std::cout<<"PASS exact original failing draw projects all triangles with finite positions and texture coordinates\n";return 0;
 }catch(const std::exception& error){std::cerr<<"FAIL projection replay: "<<error.what()<<"\n";return 1;}
}