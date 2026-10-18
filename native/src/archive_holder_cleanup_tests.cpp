using s32 = int;
class JKRHeap {
public:
    static void free(void*, JKRHeap*);
};
class JKRMemArchive {
public:
    void unmount();
    ~JKRMemArchive();
    bool unmounted = false;
    JKRHeap* heap = nullptr;
};

struct Allocation { void* pointer; JKRHeap* heap; };
static Allocation allocations[16]{};
static int allocationCount = 0;
static int freeCount = 0;
static int unmountCount = 0;
static int archiveDestructorCount = 0;
static int failureCode = 0;
static void check(bool condition, int code) { if (!condition && !failureCode) failureCode = code; }

void JKRHeap::free(void* pointer, JKRHeap* heap) {
    int index = 0;
    while (index < allocationCount && allocations[index].pointer != pointer) ++index;
    check(index < allocationCount, 1);
    if (index == allocationCount) return;
    check(allocations[index].heap == heap, 2);
    for (int i = index; i + 1 < allocationCount; ++i) allocations[i] = allocations[i + 1];
    --allocationCount;
    ++freeCount;
}
void JKRMemArchive::unmount() { check(!unmounted, 3); unmounted = true; ++unmountCount; this->~JKRMemArchive(); JKRHeap::free(this, heap); }
JKRMemArchive::~JKRMemArchive() { check(unmounted, 4); ++archiveDestructorCount; }

namespace MR {
static JKRHeap* aliasHeap = nullptr;
static JKRHeap* aliasOwner = nullptr;
static JKRHeap* getHeapNapa(JKRHeap* heap) { return heap == aliasOwner ? aliasHeap : nullptr; }
static JKRHeap* getHeapGDDR3(JKRHeap*) { return nullptr; }
template<class T> class Vector {
public:
    T* begin() { return values; }
    T* end() { return values + count; }
    void push_back(T value) { values[count++] = value; }
    void erase(T* position) {
        for (T* it = position; it + 1 < end(); ++it) *it = *(it + 1);
        --count;
    }
    int size() const { return count; }
    T at(int index) const { return values[index]; }
private:
    T values[8]{};
    int count = 0;
};
}

class ArchiveHolderArchiveEntry {
public:
    JKRMemArchive* mArchive = nullptr;
    JKRHeap* mHeap = nullptr;
    char* mArchiveName = nullptr;
    ~ArchiveHolderArchiveEntry();
};
class ArchiveHolder {
public:
    MR::Vector<ArchiveHolderArchiveEntry*> mEntries;
    void removeIfIsEqualHeap(JKRHeap*);
};

void operator delete(void*) noexcept {}
void operator delete(void*, unsigned __int64) noexcept {}
static void releaseArray(void* pointer) {
#ifdef GALAXY_NATIVE_ARCHIVE_CLEANUP
    // Native placement allocations must never reach host delete[].
    check(false, 40);
    (void)pointer;
#else
    int index = 0;
    while (index < allocationCount && allocations[index].pointer != pointer) ++index;
    check(index < allocationCount, 5);
    if (index < allocationCount) JKRHeap::free(pointer, allocations[index].heap);
#endif
}
void operator delete[](void* pointer) noexcept { releaseArray(pointer); }
void operator delete[](void* pointer, unsigned __int64) noexcept { releaseArray(pointer); }

// ACTUAL_METHODS_INSERTION_POINT

static int runTest() {
    JKRHeap ownerA, ownerB, ownerC, alias;
    JKRMemArchive archives[3];
    ArchiveHolderArchiveEntry entries[3];
    char names[3][16] = {{'a',0},{'b',0},{'c',0}};
    JKRHeap* owners[3] = {&ownerA, &ownerB, &ownerC};
    ArchiveHolder holder;
    for (int i = 0; i < 3; ++i) {
        entries[i].mArchive = &archives[i];
        archives[i].heap = owners[i];
        entries[i].mHeap = owners[i];
        entries[i].mArchiveName = names[i];
        holder.mEntries.push_back(&entries[i]);
        allocations[allocationCount++] = {&archives[i], owners[i]};
        allocations[allocationCount++] = {names[i], owners[i]};
        allocations[allocationCount++] = {&entries[i], owners[i]};
    }

    MR::aliasHeap = &alias;
    MR::aliasOwner = &ownerB;
    holder.removeIfIsEqualHeap(&alias);
    check(holder.mEntries.size() == 2, 10);
    check(holder.mEntries.at(0) == &entries[0] && holder.mEntries.at(1) == &entries[2], 11);
    check(freeCount == 3 && unmountCount == 1 && archiveDestructorCount == 1, 12);

    holder.removeIfIsEqualHeap(&ownerA);
    check(holder.mEntries.size() == 1 && holder.mEntries.at(0) == &entries[2], 20);
    check(freeCount == 6 && unmountCount == 2 && archiveDestructorCount == 2, 21);
    holder.removeIfIsEqualHeap(&ownerA);
    check(holder.mEntries.size() == 1 && freeCount == 6, 22);

    holder.removeIfIsEqualHeap(&ownerC);
    check(holder.mEntries.size() == 0 && allocationCount == 0 && freeCount == 9, 30);
    check(unmountCount == 3 && archiveDestructorCount == 3, 31);
    return failureCode;
}

extern "C" __declspec(dllimport) void ExitProcess(unsigned int);
extern "C" void mainCRTStartup() { ExitProcess(static_cast<unsigned int>(runTest())); }
