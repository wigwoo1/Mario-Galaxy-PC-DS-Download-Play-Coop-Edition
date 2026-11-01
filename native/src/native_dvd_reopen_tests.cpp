#include "native_dvd_port.hpp"
#include <revolution/dvd.h>
#include <cassert>
#include <cstdio>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <array>
#include <algorithm>
namespace {
std::mutex mutex;std::condition_variable changed;bool entered=false,release=false;unsigned callbacks=0;
void heldCallback(s32 bytes,DVDFileInfo*){
    assert(bytes==64);std::unique_lock<std::mutex> lock(mutex);++callbacks;entered=true;changed.notify_one();
    assert(changed.wait_for(lock,std::chrono::seconds(10),[]{return release;}));
}
}
int main(int argc,char**argv){
    assert(argc==2);assert(nativeBindDvd(std::filesystem::u8path(argv[1])));
    const int track=DVDConvertPathToEntrynum("/AudioRes/Stream/SMG_ev_prolo04_strm.ast"),font=DVDConvertPathToEntrynum("/LayoutData/Font.arc");
    assert(track>0&&font>0);DVDFileInfo info{};std::array<u8,64> output{};
    std::vector<u8> trackBytes,fontBytes;assert(nativeReadDvdEntry(track,trackBytes)&&nativeReadDvdEntry(font,fontBytes));
    assert(DVDFastOpen(track,&info)&&DVDFastOpen(track,&info));
    assert(DVDReadPrio(&info,output.data(),64,0,2)==64&&std::equal(output.begin(),output.end(),trackBytes.begin()));
    assert(DVDFastOpen(font,&info)&&info.length==fontBytes.size());
    assert(DVDReadPrio(&info,output.data(),64,0,2)==64&&std::equal(output.begin(),output.end(),fontBytes.begin()));
    const auto oldAddress=info.startAddr;assert(!DVDFastOpen(-1,&info)&&!DVDFastOpen(0,&info)&&info.startAddr==oldAddress&&info.length==fontBytes.size());
    std::puts("PASS SDK idle file-info reuse: same/different FST entries return exact bytes, invalid entry preserves prior binding");
    assert(DVDReadAsyncPrio(&info,output.data(),64,0,heldCallback,2));
    {std::unique_lock<std::mutex> lock(mutex);assert(changed.wait_for(lock,std::chrono::seconds(10),[]{return entered;}));}
    assert(!DVDFastOpen(track,&info)&&info.length==fontBytes.size()&&info.startAddr==oldAddress);
    {std::lock_guard<std::mutex> lock(mutex);release=true;changed.notify_one();}
    assert(DVDClose(&info)&&callbacks==1&&std::equal(output.begin(),output.end(),fontBytes.begin()));
    assert(DVDFastOpen(track,&info)&&info.length==trackBytes.size());assert(DVDClose(&info));
    nativeReleaseDvd();std::puts("PASS pending native read/callback owns metadata until completion; reopen rejected without mutation, close synchronizes and subsequent reuse succeeds");
}
