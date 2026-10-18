#pragma once
#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>
namespace Native {
// Metadata access is serialized across host threads. Registrations own snapshots; refresh updates bytes at
// stable addresses. Addresses are never reused within a registry lifetime.
class GpuResources {
    struct Range {
        std::uintptr_t host=0;
        std::uint32_t address=0;
        std::vector<std::uint8_t> bytes;
        const std::uint8_t* borrowed=nullptr;
        std::size_t borrowedSize=0;
        std::shared_ptr<const void> lifetime;
        bool nativeWords=false;
        bool writable=false;
        std::size_t size()const{return borrowed?borrowedSize:bytes.size();}
        const std::uint8_t* data()const{return borrowed?borrowed:bytes.data();}
    };
    std::vector<Range> ranges;
    std::uint64_t next=32, revision=0;
    std::uint32_t addOwned(const void*,std::size_t,bool nativeWords,bool writable);
public:
    std::uint64_t generation() const;
    std::uint32_t add(const void*,std::size_t);
    // Register live data owned elsewhere without taking a second copy. The
    // shared owner keeps the host bytes alive until this mapping is removed.
    std::uint32_t addBorrowed(const void*,std::size_t,std::shared_ptr<const void> lifetime,bool writable=false);
    std::uint32_t addWritable(void*,std::size_t);
    void write(std::uint32_t,const void*,std::size_t);
    std::uint32_t addNativeWords(const void*,std::size_t);
    void remove(const void*);
    // Refresh intersections with registered live host storage without changing
    // command addresses. Registrations must be removed before storage dies.
    void refresh(const void*,std::size_t);
    std::uint32_t address(const void*) const;
    std::uint32_t byteAddress(const void*) const;
    const std::uint8_t* resolve(std::uint32_t,std::size_t) const;
    // SDK getters return the original CPU address, not the render snapshot.
    // The registered owner must remain alive while callers retain this pointer.
    const std::uint8_t* hostAddress(std::uint32_t,std::size_t) const;
};
GpuResources& gpuResources();
// Map the initialized, readable data sections of the module containing anchor.
// This covers static GX assets while keeping reads bounded to PE section extents.
void registerExecutableImageResources(const void* anchor);
}



