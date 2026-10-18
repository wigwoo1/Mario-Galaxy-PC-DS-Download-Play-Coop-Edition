#include <Native/GpuResources.hpp>
#include <array>
#include <atomic>
#include <thread>
#include <vector>
#include <cassert>
#include <cstring>
#include <cstdio>
int main(){
 Native::GpuResources resources;std::vector<unsigned char> framebuffer(65536),pixels(65536);
 auto address=resources.addWritable(framebuffer.data(),framebuffer.size());std::atomic<bool> start{false};
 std::thread registrar([&]{while(!start.load(std::memory_order_acquire))std::this_thread::yield();
  std::vector<std::array<unsigned char,64>> assets(4096);std::vector<const void*> active;
  for(unsigned i=0;i<assets.size();++i){resources.add(assets[i].data(),assets[i].size());active.push_back(assets[i].data());if(i%3==0&&active.size()>1){resources.remove(active.front());active.erase(active.begin());}}
  for(auto* block:active)resources.remove(block);
 });
 start.store(true,std::memory_order_release);
 auto generation=resources.generation();
 for(unsigned frame=0;frame<2000;++frame){std::memset(pixels.data(),frame&255,pixels.size());resources.write(address,pixels.data(),pixels.size());
  assert(resources.byteAddress(framebuffer.data())==address);assert(!std::memcmp(framebuffer.data(),pixels.data(),pixels.size()));
  assert(!std::memcmp(resources.resolve(address,pixels.size()),pixels.data(),pixels.size()));
  auto next=resources.generation();assert(next>generation);generation=next;
 }
 registrar.join();resources.remove(framebuffer.data());
 std::puts("PASS concurrent resource registry: 4096 asset registrations/removals, 2000 framebuffer writes, stable addresses and exact host/snapshot bytes");
}