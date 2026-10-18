#pragma once
#include <array>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <vector>
#include <utility>
namespace Native {
// Owns host-endian data; callers must provide the actual archive member length.
struct CameraAnimationData {
    struct Component { uint32_t count, offset, type; };
    bool keyed{};
    uint32_t frames{};
    std::array<uint32_t,4> metadata{};
    std::array<Component, 8> components{};
    std::vector<float> values;
    // Rebuild the original pointer-free file layout in host byte order.
    // Consumers must retain this owned vector for the lifetime of their accessors.
    std::vector<uint8_t> hostFile() const {
        if(!frames || values.empty()) return {};
        const size_t infoBytes=keyed?96:64;
        std::vector<uint8_t> file(32+infoBytes+4+values.size()*4);
        std::memcpy(file.data(),keyed?"ANDOCKAN":"ANDOCANM",8);
        auto store=[&file](size_t p,uint32_t v){std::memcpy(file.data()+p,&v,4);};
        for(size_t i=0;i<4;++i)store(8+i*4,metadata[i]);
        store(24,frames);store(28,static_cast<uint32_t>(infoBytes));
        for(size_t i=0;i<8;++i){
            const size_t p=32+i*(keyed?12:8);
            store(p,components[i].count);store(p+4,components[i].offset);
            if(keyed)store(p+8,components[i].type);
        }
        store(32+infoBytes,static_cast<uint32_t>(values.size()*4));
        std::memcpy(file.data()+36+infoBytes,values.data(),values.size()*4);
        return file;
    }
    bool decode(const uint8_t* data, size_t size) {
        if (!data || size < 32 || std::memcmp(data,"ANDO",4)) return false;
        const bool key = !std::memcmp(data+4,"CKAN",4);
        if (!key && std::memcmp(data+4,"CANM",4)) return false;
        auto word = [data](size_t p) { return uint32_t(data[p])<<24 | uint32_t(data[p+1])<<16 | uint32_t(data[p+2])<<8 | data[p+3]; };
        const size_t infoBytes = key ? 96 : 64;
        const size_t offset = word(28);
        if (!word(8) || !word(24) || offset < infoBytes || offset > size-32 || size-32-offset < 4) return false;
        const size_t bytes = word(32+offset);
        if (bytes%4 || bytes > size-32-offset-4) return false;
        CameraAnimationData next;
        next.keyed=key; next.frames=word(24);
        for(size_t i=0;i<4;++i) next.metadata[i]=word(8+i*4);
        next.values.resize(bytes/4);
        for(size_t i=0;i<next.values.size();++i) {
            uint32_t bits=word(36+offset+i*4);
            std::memcpy(&next.values[i],&bits,4);
            if(!std::isfinite(next.values[i])) return false;
        }
        for(size_t i=0;i<8;++i) {
            const size_t p=32+i*(key?12:8);
            auto& c=next.components[i];
            c={word(p),word(p+4),key?word(p+8):0};
            if(!c.count || c.type>1 || c.offset>next.values.size()) return false;
            const size_t stride=key && c.count>1 ? (c.type?4:3) : 1;
            if(c.count>(next.values.size()-c.offset)/stride) return false;
            if(key && c.count>1) {
                for(size_t j=1;j<c.count;++j)
                    if(next.values[c.offset+j*stride]<=next.values[c.offset+(j-1)*stride]) return false;
            }
        }
        *this=std::move(next);
        return true;
    }
};
}

