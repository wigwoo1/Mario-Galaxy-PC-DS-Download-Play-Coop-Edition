#pragma once
#include "native_render_stream.hpp"
#include <Native/D3dModelDraw.hpp>
#include <filesystem>
#include <fstream>
#include <cstdlib>
#include <iomanip>

// Diagnostic-only observer used by the boot-verification header copy. It never
// changes GX state or render targets. Readbacks run for one selected XFB frame.
class NativeFramebufferProbe {
 struct State {
  unsigned displays=0,target=640,readyDelay=2,index=0,savedChanges=0;bool started=false,readyMode=false,menuMode=false,armed=false;
  std::filesystem::path directory;std::ofstream log;std::vector<std::uint8_t> before;
  State(){if(auto* path=std::getenv("GALAXY_FRAMEBUFFER_PROBE_DIR")){directory=std::filesystem::u8path(path);std::filesystem::create_directories(directory);if(auto* frame=std::getenv("GALAXY_FRAMEBUFFER_PROBE_FRAME"))target=unsigned(std::strtoul(frame,nullptr,10));if(auto* delay=std::getenv("GALAXY_FRAMEBUFFER_PROBE_READY_DELAY")){char* end=nullptr;auto value=std::strtoul(delay,&end,10);if(end==delay||*end||value<1||value>100000)throw std::invalid_argument("Framebuffer ready delay must be 1..100000");readyDelay=unsigned(value);}menuMode=std::getenv("GALAXY_FRAMEBUFFER_PROBE_MENU_READY")!=nullptr;readyMode=menuMode||std::getenv("GALAXY_FRAMEBUFFER_PROBE_READY")!=nullptr;log.open(directory/"operations.tsv");}}
 };
 static State& state(){static State value;return value;}
 Native::D3dEfb& efb;bool active=false;
 static void put32(std::ofstream& file,std::uint32_t value){for(unsigned i=0;i<4;++i)file.put(char(value>>(i*8)));}
 static void put16(std::ofstream& file,unsigned value){file.put(char(value));file.put(char(value>>8));}
 void save(const std::vector<std::uint8_t>& rgba,const std::string& name){
  auto& s=state();std::ofstream file(s.directory/(name+".bmp"),std::ios::binary);const unsigned w=efb.getWidth(),h=efb.getHeight();
  file.put('B');file.put('M');put32(file,54+w*h*4);put32(file,0);put32(file,54);put32(file,40);put32(file,w);put32(file,h);put16(file,1);put16(file,32);put32(file,0);put32(file,w*h*4);for(unsigned i=0;i<4;++i)put32(file,0);
  for(unsigned y=h;y-->0;)for(unsigned x=0;x<w;++x){const auto i=(std::size_t(y)*w+x)*4;file.put(char(rgba[i+2]));file.put(char(rgba[i+1]));file.put(char(rgba[i]));file.put(char(rgba[i+3]));}
 }
public:
 explicit NativeFramebufferProbe(Native::D3dEfb& value):efb(value){}
 static void armSceneFrame(bool menuEvent=false){auto& s=state();if(s.menuMode!=menuEvent)return;if(s.log.is_open()&&s.readyMode&&!s.armed){s.armed=true;s.target=s.displays+s.readyDelay;s.log<<(menuEvent?"MENU READY displays=":"SCENE READY displays=")<<s.displays<<" selected="<<s.target<<'\n';s.log.flush();}}
 void before(const NativeRenderOperation& op,const std::vector<Native::ProjectedVertex>* vertices){
  auto& s=state();active=s.log.is_open()&&(!s.readyMode||s.armed)&&s.displays+1==s.target;
  if(!active)return;
  s.before=efb.readColor();
  if(!s.started){save(s.before,"frame-start");s.started=true;s.log<<"index\tkind\tchangedRGBA\tbluePixels\tcenterRGBA\tmode41\tzmode\tcull\talphaTest\tvertices\tinsideClip\tfirstClipXYZW\tviewport\tscissor\tcopyMode\tclearAR_GB\n";}
  s.log<<s.index<<'\t'<<unsigned(op.kind)<<'\t';
  if(op.kind==NativeRenderOperation::Kind::Copy)save(s.before,"op-"+std::to_string(s.index)+"-before-copy");
  if(vertices){
   unsigned inside=0;for(const auto& v:*vertices){auto* p=v.position;if(p[3]>0&&p[0]>=-p[3]&&p[0]<=p[3]&&p[1]>=-p[3]&&p[1]<=p[3]&&p[2]>=-p[3]&&p[2]<=0)++inside;}
   clipInside=inside;first=vertices->empty()?Native::ProjectedVertex{}:(*vertices)[0];vertexCount=vertices->size();
  }else{clipInside=0;vertexCount=0;first={};}
 }
 void after(const NativeRenderOperation& op){
  auto& s=state();
  if(active){
   auto pixels=efb.readColor();std::size_t changed=0,blue=0;
   for(std::size_t i=0;i<pixels.size();i+=4){if(std::memcmp(pixels.data()+i,s.before.data()+i,4))++changed;if(pixels[i]==30&&pixels[i+1]==30&&pixels[i+2]==200)++blue;}
   const auto center=(std::size_t(efb.getHeight()/2)*efb.getWidth()+efb.getWidth()/2)*4;
   s.log<<changed<<'\t'<<blue<<'\t'<<std::hex<<unsigned(pixels[center])<<','<<unsigned(pixels[center+1])<<','<<unsigned(pixels[center+2])<<','<<unsigned(pixels[center+3])<<'\t'<<op.state.bp(0x41)<<'\t'<<op.state.bp(0x40)<<'\t'<<((op.state.bp(0)>>14)&3)<<'\t'<<op.state.bp(0xf3)<<std::dec<<'\t'<<vertexCount<<'\t'<<clipInside<<'\t';
   for(float v:first.position)s.log<<v<<',';
   s.log<<'\t';for(auto v:op.state.viewport())s.log<<v<<',';s.log<<'\t';for(auto v:op.state.scissorRectangle())s.log<<v<<',';
   s.log<<'\t'<<std::hex<<op.copy.registers[0x52]<<'\t'<<op.copy.registers[0x4f]<<','<<op.copy.registers[0x50]<<std::dec<<'\n';s.log.flush();
   if(changed&&s.savedChanges<8){save(pixels,"op-"+std::to_string(s.index)+"-changed");++s.savedChanges;}
   if(op.kind==NativeRenderOperation::Kind::Copy)save(pixels,"op-"+std::to_string(s.index)+"-after-copy");
   ++s.index;
  }
  if(op.kind==NativeRenderOperation::Kind::Copy&&op.copy.display()){++s.displays;if(active){s.log<<"FRAME COMPLETE\n";s.log.flush();}}
 }
private:
 unsigned clipInside=0;std::size_t vertexCount=0;Native::ProjectedVertex first{};
};
