#ifndef OSMUTEX_H
#define OSMUTEX_H

#ifdef __cplusplus
extern "C" {
#endif

#include "revolution/os/OSThread.h"

struct OSMutex {
    OSThreadQueue queue;
    OSThread* thread;
    s32 count;
    OSMutexLink link;
};

struct OSCond {
    OSThreadQueue queue;
};

// The native host mutex adapter mirrors the retail-visible owner and recursion
// count fields while keeping the actual host synchronization state out of the
// retail ABI object.
#define GALAXY_OSMUTEX_HAS_LAYOUT 1

void OSInitMutex(OSMutex *);
void OSLockMutex(OSMutex *);
void OSUnlockMutex(OSMutex *);
BOOL OSTryLockMutex(OSMutex *);

#ifdef __cplusplus
}
#endif

#endif // OSMUTEX_H
