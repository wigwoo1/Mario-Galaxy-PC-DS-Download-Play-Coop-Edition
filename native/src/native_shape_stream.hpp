#pragma once
#include "native_shape_vertices.hpp"
#include <functional>
inline void replayShapeStream(Native::GxCommandState& state,const u8* bytes,size_t size,const Native::GpuResources& resources,
 const std::function<void(const std::vector<NativeShapeVertex>&,const std::vector<std::array<u32,3>>&)>& submit,unsigned depth=0){
    assetCheck(depth<16,"Shape command nesting exceeded");size_t p=0;
    while(p<size){u8 op=bytes[p];if(!op){++p;continue;}size_t length=0;
        if(op==0x40){extent(p,9,size);u32 address=be32(bytes+p+1),count=be32(bytes+p+5);auto* nested=resources.resolve(address,count);
            size_t first=0;while(first<count && !nested[first])++first;
            if(first<count && (nested[first]&0x80)){std::vector<std::array<u32,3>> triangles;auto vertices=decodeShapePositions(state,nested,count,resources,&triangles);submit(vertices,triangles);}
            else replayShapeStream(state,nested,count,resources,submit,depth+1);p+=9;continue;
        }
        if(op==8)length=6;else if(op==0x61)length=5;else if(op==0x10){extent(p,5,size);length=5+(size_t(bytes[p+1])*256+bytes[p+2]+1)*4;}
        else if(op==0x20 || op==0x28 || op==0x30 || op==0x38)length=5;
        else assetCheck(false,"Unsupported shape setup command");extent(p,length,size);state.consume(bytes+p,length,&resources);p+=length;
    }
}
