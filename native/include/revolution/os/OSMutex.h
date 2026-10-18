#pragma once
#include <revolution/types.h>
struct OSThread;
struct OSMutex { u32 retailStorage[6]; };
#ifdef __cplusplus
extern "C" {
#endif
void OSInitMutex(OSMutex*);
void OSLockMutex(OSMutex*);
void OSUnlockMutex(OSMutex*);
BOOL OSTryLockMutex(OSMutex*);
#ifdef __cplusplus
}
#endif
