#pragma once
#include "rig.hpp"
#include "asset_tests.hpp"
inline void rigTests(){
    std::vector<uint8_t> b(232,0);std::memcpy(b.data(),"J3D2bdl4",8);put32(b,8,232);put32(b,12,2);
    std::memcpy(b.data()+32,"INF1",4);put32(b,36,44);put32(b,52,24);
    b[57]=16;b[61]=1;b[65]=16;b[67]=1;b[69]=2;
    size_t j=76;std::memcpy(b.data()+j,"JNT1",4);put32(b,j+4,156);b[j+9]=2;put32(b,j+12,24);put32(b,j+16,152);b[j+155]=1;
    for(size_t i=0;i<2;++i)for(size_t k=0;k<3;++k)put32(b,j+24+i*64+4+k*4,0x3f800000);
    put32(b,j+24+64+24,0x40000000);
    auto r=decodeRig(BinaryView(b));assetCheck(r.joints.size()==2&&r.joints[0].parent==-1&&r.joints[1].parent==0&&r.joints[1].translation[0]==2,"Joint hierarchy/transform test failed");
    auto rejects=[&](std::vector<uint8_t> invalid){bool caught=false;try{decodeRig(BinaryView(invalid));}catch(const std::runtime_error&){caught=true;}assetCheck(caught,"Invalid rig accepted");};
    auto bad=b;bad[61]=2;rejects(bad);
    bad=b;bad[67]=0;rejects(bad);
    bad=b;bad[j+155]=2;rejects(bad);
    bad=b;put32(bad,j+28,0x7f800000);rejects(bad);
    bad=b;bad[69]=17;rejects(bad);
}
