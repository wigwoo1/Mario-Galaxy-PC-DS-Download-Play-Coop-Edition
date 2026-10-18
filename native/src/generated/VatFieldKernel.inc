static inline void SETVAT(u32* va, u32* vb, u32* vc, GXAttr attr, GXCompCnt cnt, GXCompType type, u8 shft) {
    switch (attr) {
    case GX_VA_POS:
        nativeVatField(*va,cnt,0,1);
        nativeVatField(*va,type,1,3);
        nativeVatField(*va,shft,4,5);
        break;
    case GX_VA_NRM:
    case GX_VA_NBT:
        nativeVatField(*va,type,10,3);
        if (cnt == GX_NRM_NBT3) {
            nativeVatField(*va,GX_NRM_NBT,9,1);
            nativeVatField(*va,1,31,1);
        } else {
            nativeVatField(*va,cnt,9,1);
            nativeVatField(*va,0,31,1);
        }
        break;
    case GX_VA_CLR0:
        nativeVatField(*va,cnt,13,1);
        nativeVatField(*va,type,14,3);
        break;
    case GX_VA_CLR1:
        nativeVatField(*va,cnt,17,1);
        nativeVatField(*va,type,18,3);
        break;
    case GX_VA_TEX0:
        nativeVatField(*va,cnt,21,1);
        nativeVatField(*va,type,22,3);
        nativeVatField(*va,shft,25,5);
        break;
    case GX_VA_TEX1:
        nativeVatField(*vb,cnt,0,1);
        nativeVatField(*vb,type,1,3);
        nativeVatField(*vb,shft,4,5);
        break;
    case GX_VA_TEX2:
        nativeVatField(*vb,cnt,9,1);
        nativeVatField(*vb,type,10,3);
        nativeVatField(*vb,shft,13,5);
        break;
    case GX_VA_TEX3:
        nativeVatField(*vb,cnt,18,1);
        nativeVatField(*vb,type,19,3);
        nativeVatField(*vb,shft,22,5);
        break;
    case GX_VA_TEX4:
        nativeVatField(*vb,cnt,27,1);
        nativeVatField(*vb,type,28,3);
        nativeVatField(*vc,shft,0,5);
        break;
    case GX_VA_TEX5:
        nativeVatField(*vc,cnt,5,1);
        nativeVatField(*vc,type,6,3);
        nativeVatField(*vc,shft,9,5);
        break;
    case GX_VA_TEX6:
        nativeVatField(*vc,cnt,14,1);
        nativeVatField(*vc,type,15,3);
        nativeVatField(*vc,shft,18,5);
        break;
    case GX_VA_TEX7:
        nativeVatField(*vc,cnt,23,1);
        nativeVatField(*vc,type,24,3);
        nativeVatField(*vc,shft,27,5);
        break;
    default:
        break;
    }
}

