#pragma once
#include <cstddef>
class J3DAnmBase;
class JKRArchive;
namespace Native {
// Returned object owns all converted BCK tables; delete through virtual base.
J3DAnmBase* createBckAnimation(const void* data,std::size_t size);
J3DAnmBase* loadArchiveBck(const JKRArchive& archive,const void* data);
// BCA (full transform) animations need the same owned, byte-order-safe host
// representation as BCK key animations when read from Wii archives.
J3DAnmBase* createBcaAnimation(const void* data,std::size_t size);
J3DAnmBase* loadArchiveBca(const JKRArchive& archive,const void* data);
J3DAnmBase* createBpkAnimation(const void* data,std::size_t size);
J3DAnmBase* loadArchiveBpk(const JKRArchive& archive,const void* data);
}
