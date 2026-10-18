#include <Native/XfbResourceRegistry.hpp>
#include <cassert>
#include <cstdio>
#include <stdexcept>

int main() {
    Native::GpuResources resources;
    Native::XfbResourceRegistry registry;
    alignas(32) std::array<std::uint8_t,128> a{},b{},c{};
    registry.refresh(resources,{a.data(),b.data(),nullptr},2,16,4);
    assert(registry.registeredCount()==2);
    const std::uint8_t value=0x5a;
    resources.write(resources.address(a.data()),&value,1);
    assert(a[0]==value);

    registry.refresh(resources,{a.data(),b.data(),nullptr},2,16,4);
    assert(registry.registeredCount()==2);
    registry.refresh(resources,{c.data(),b.data(),nullptr},2,16,4);
    bool oldRemoved=false;try{(void)resources.address(a.data());}catch(const std::invalid_argument&){oldRemoved=true;}
    assert(oldRemoved&&resources.address(c.data()));
    registry.clear(resources);
    bool currentRemoved=false;try{(void)resources.address(c.data());}catch(const std::invalid_argument&){currentRemoved=true;}
    assert(currentRemoved&&registry.registeredCount()==0);

    alignas(32) std::array<std::uint8_t,128> externallyOwned{};
    resources.addWritable(externallyOwned.data(),externallyOwned.size());
    registry.refresh(resources,{externallyOwned.data(),nullptr,nullptr},1,16,4);
    registry.clear(resources);
    assert(resources.address(externallyOwned.data()));
    resources.remove(externallyOwned.data());
    std::puts("PASS JUTXfb guest-resource registration, refresh, replacement and external ownership");
}
