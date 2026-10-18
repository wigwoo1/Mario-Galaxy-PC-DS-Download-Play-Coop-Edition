#pragma once
#include <revolution/mtx.h>
namespace Native {
// World joint * inverse bind, accumulated in original influence order.
inline u8 envelopeMatrix(const Mtx* world,const Mtx* inverse,const u16* indices,
                         const f32* weights,const u8* flags,unsigned count,Mtx result){
    for(int r=0;r<3;++r)for(int c=0;c<4;++c)result[r][c]=0;
    u8 scaleFlag=1;
    for(unsigned j=0;j<count;++j){
        const auto index=indices[j];
        const auto& a=world[index];const auto& b=inverse[index];
        for(int r=0;r<3;++r)for(int c=0;c<4;++c){
            const f32 composed=a[r][0]*b[0][c]+a[r][1]*b[1][c]+a[r][2]*b[2][c]+(c==3?a[r][3]:0);
            result[r][c]+=composed*weights[j];
        }
        scaleFlag&=flags[index];
    }
    return scaleFlag;
}
}
