#include "assets.hpp"
#include "native_dvd_port.hpp"
#include <JSystem/JKernel/JKRDvdFile.hpp>
#include <cassert>
#include <cstdio>
#include <thread>
#include <JSystem/JKernel/JKRExpHeap.hpp>
#include <new>
int main(int argc,char** argv){assert(argc==2);auto path=std::filesystem::u8path(argv[1]);WiiAssets reference(path);auto expected=reference.readFile("/LayoutData/Font.arc");assert(nativeBindDvd(path));{JKRDvdFile file;assert(file.open("/LayoutData/Font.arc"));assert(file.getFileSize()==expected.size());std::vector<uint8_t> bytes(expected.size());assert(file.readData(bytes.data(),s32(bytes.size()),0)==s32(bytes.size()));assert(bytes==expected);for(int i=0;i<16;++i){uint8_t chunk[32];assert(file.readData(chunk,32,i*32)==32);assert(std::equal(chunk,chunk+32,expected.data()+i*32));}assert(file.getStatus()==0);assert(file.writeData(bytes.data(),32,0)==-1);file.close();assert(!file.isAvailable());}{JKRDvdFile a,b;assert(a.open("/LayoutData/Font.arc"));assert(b.open("/LayoutData/Font.arc"));auto run=[&](JKRDvdFile& file){for(int i=0;i<1000;++i){uint8_t chunk[32];int offset=(i*32)%65536;assert(file.readData(chunk,32,offset)==32);assert(std::equal(chunk,chunk+32,expected.data()+offset));}};std::thread first([&]{run(a);});std::thread second([&]{run(b);});first.join();second.join();a.close();b.close();}{alignas(32) unsigned char storage[65536]{}; JKRExpHeap parent(storage,sizeof(storage),nullptr,false); JKRHeap::sRootHeap=&parent; parent.becomeCurrentHeap(); auto initial=parent.getTotalFreeSize(); for(int i=0;i<32;++i){void* memory=JKRHeap::alloc(sizeof(JKRDvdFile),32,&parent); assert(memory); auto* file=new(memory) JKRDvdFile; assert(file->open("/LayoutData/Font.arc")); delete file; assert(parent.check()); assert(parent.getTotalFreeSize()==initial);} assert(JKRDvdFile::sDvdList.getNumLinks()==0);} nativeReleaseDvd();std::puts("PASS original JKRDvdFile retail bytes, 2016 partial reads including concurrent file queues, status and close");}



