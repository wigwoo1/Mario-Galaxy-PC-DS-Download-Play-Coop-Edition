#include <JSystem/JUtility/JUTTexture.hpp>

void JUTTexture::captureDolTexture(void* buffer,int width,int height,int x,int y,bool mipmap,GXTexFmt format){
 if(mipmap)GXSetTexCopySrc(x,y,width*2,height*2);
 else GXSetTexCopySrc(x,y,width,height);
 GXSetTexCopyDst(width,height,format,mipmap);
 GXCopyTex(buffer,GX_FALSE);
 GXPixModeSync();
}
