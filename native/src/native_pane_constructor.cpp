#include <nw4r/lyt/pane.h>
namespace nw4r::lyt {
// Native default for programmatically created pictures; resource-backed panes
// continue to use the original constructor and their resource values.
Pane::Pane() {
    Init();
    mTranslate={0.0f,0.0f,0.0f}; mRotate={0.0f,0.0f,0.0f};
    mScale={1.0f,1.0f}; mSize={0.0f,0.0f};
    mMtx={}; mGlbMtx={};
    for(int i=0;i<3;++i){mMtx.m[i][i]=1.0f;mGlbMtx.m[i][i]=1.0f;}
    mAlpha=255; mGlbAlpha=255; mBasePosition=0; mFlag=1;
    mName[0]=0; mUserData[0]=0; mPadding=0;
}
}
