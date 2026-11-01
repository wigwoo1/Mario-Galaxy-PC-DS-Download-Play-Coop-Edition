#pragma once
#include <revolution/os.h>
// Callback harness only: uses the real SDK message queue, not a running JKR
// audio thread. Its DSPCallback method body is extracted unchanged from JAudio,
// with only the enclosing class name adapted by the test generator.
class TestAudioThread {
public:
    static TestAudioThread* current;
    OSMessageQueue queue{};OSMessage messages[32]{};
    static TestAudioThread* getInstance(){return current;}
    TestAudioThread(){current=this;OSInitMessageQueue(&queue,messages,32);}
    bool sendMessage(void* message){return OSSendMessage(&queue,message,OS_MESSAGE_NOBLOCK);}
    static void DSPCallback(void*);
};
enum {AUDIOMSG_DSP=1};
