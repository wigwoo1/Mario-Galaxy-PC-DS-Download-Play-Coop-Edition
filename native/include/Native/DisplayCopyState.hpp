#pragma once
#include <revolution/types.h>
namespace Native {
struct DisplayCopyState {u32 cpDispSrc=0,cpDispSize=0,cpDispStride=0,cpDisp=0;};
const DisplayCopyState& displayCopyState();
}
