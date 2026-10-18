using AllocationSize = decltype(sizeof(0));
void* operator new(AllocationSize, void* storage) noexcept { return storage; }

class JKRHeap {
public:
    static JKRHeap* findFromRoot(void*);
    static void free(void*, JKRHeap*);
};
static int failure, destructions, releases, hostDeletes;
static void* allocation;
static JKRHeap* owner;
JKRHeap* JKRHeap::findFromRoot(void* pointer) { return pointer == allocation ? owner : nullptr; }
void JKRHeap::free(void* pointer, JKRHeap* heap) {
    if (pointer != allocation || heap != owner || destructions != 1) failure = 1;
    ++releases;
}
class JKRFileLoader {
public:
    virtual ~JKRFileLoader() {}
    void unmount();
    unsigned int _34 = 0;
};
class Archive : public JKRFileLoader {
public:
    ~Archive() override { ++destructions; }
};
void operator delete(void*) noexcept { ++hostDeletes; failure = 2; }
void operator delete(void*, AllocationSize) noexcept { ++hostDeletes; failure = 2; }

// ACTUAL_UNMOUNT_INSERTION_POINT

static int runTest() {
    JKRHeap heap;
    alignas(Archive) unsigned char storage[sizeof(Archive)];
    Archive* archive = new (storage) Archive();
    allocation = archive;
    owner = &heap;
    archive->unmount();
    if (releases || destructions || archive->_34) return 3;
    archive->_34 = 2;
    archive->unmount();
    if (releases || destructions || archive->_34 != 1) return 4;
    archive->unmount();
    if (releases != 1 || destructions != 1 || hostDeletes) return 5;
    // Do not touch the destroyed object. ExitProcess skips local teardown.
    return failure;
}
extern "C" __declspec(dllimport) void ExitProcess(unsigned int);
extern "C" void mainCRTStartup() { ExitProcess(static_cast<unsigned int>(runTest())); }
