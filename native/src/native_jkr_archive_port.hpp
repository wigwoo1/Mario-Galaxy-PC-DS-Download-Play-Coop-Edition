#pragma once
#include <cstddef>
class JKRMemArchive;
bool nativeRegisterRarc(const void*,std::size_t);
void nativeUnregisterRarc(const void*);
bool nativeAttachRarc(JKRMemArchive*,const void*);
void nativeDetachRarc(JKRMemArchive*);
class JKRArchive;
bool nativeAttachRarcMetadata(JKRArchive*,const void*,std::size_t* payloadOffset=nullptr);
void nativeDetachRarcMetadata(JKRArchive*);
#include <memory>
struct NativeRarcTables;
std::shared_ptr<const NativeRarcTables> nativeOwnedRarcTables(JKRArchive*);
