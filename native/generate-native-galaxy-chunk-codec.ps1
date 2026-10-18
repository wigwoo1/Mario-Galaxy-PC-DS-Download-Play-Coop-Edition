$ErrorActionPreference='Stop'
$n=$PSScriptRoot
& "$n\generate-native-galaxy-record-codec.ps1"
$s=Get-Content "$n\src\generated\GameDataGalaxyStorage.cpp" -Raw
$helpers=@"
#include <vector>
#include <cstring>
namespace {
u16 galaxyRead16(const u8* p){return u16((u16(p[0])<<8)|p[1]);}
bool validGalaxyHeader(const u8* p,u32 bytes){
 if(!p||bytes<4)return false;
 const u32 count=galaxyRead16(p),length=galaxyRead16(p+2);
 if(count>(bytes-4)/4||length==0)return false;
 for(u32 i=0;i<count;++i){
  const u16 hash=galaxyRead16(p+4+i*4),offset=galaxyRead16(p+6+i*4);
  if(offset>=length)return false;
  for(u32 j=0;j<i;++j)if(galaxyRead16(p+4+j*4)==hash)return false;
 }
 return true;
}
}
"@
$s=$helpers+[Environment]::NewLine+$s
$s=$s.Replace('*static_cast< u16* >(pData) = value;','u8* bytes=static_cast<u8*>(pData); bytes[0]=u8(value>>8); bytes[1]=u8(value);')
$a=$s.IndexOf('s32 GameDataAllGalaxyStorage::deserialize(');$b=$s.IndexOf('void GameDataAllGalaxyStorage::initializeData()', $a)
$decode=@"
s32 GameDataAllGalaxyStorage::deserialize(const u8* data,u32 size){
 if(!data||size<6||!validGalaxyHeader(data+2,size-2))return -1;
 const u32 records=galaxyRead16(data);
 BinaryDataContentAccessor accessor(const_cast<u8*>(data+2));
 const u32 header=accessor.getHeaderSize()+2,stride=accessor.getDataSize();
 if(header>size||records>(size-header)/stride)return -1;
 std::vector<GameDataSomeGalaxyStorage> pending;
 std::vector<int> indices;
 bool readError=false;
 for(u32 i=0;i<records;++i){
  const u8* record=data+header+i*stride;
  const u8* name=(u8*)accessor.getPointer("mGalaxyName",const_cast<u8*>(record));
  if(!name||stride<2||name-record>stride-2)return -1;
  const int index=findIndex(galaxyRead16(name));
  if(index<0){readError=true;continue;}
  for(int old:indices)if(old==index)return -1;
  pending.push_back(*mSomeGalaxyStorages[index]);indices.push_back(index);
  const int result=pending.back().deserialize(accessor,record);
  if(result<0)return -1;
  if(result==1)readError=true;
 }
 for(size_t i=0;i<pending.size();++i)*mSomeGalaxyStorages[indices[i]]=pending[i];
 return readError?1:0;
}

"@
$s=$s.Substring(0,$a)+$decode+$s.Substring($b)
$anchor='    // https://decomp.me/scratch/NCESx'
$guard=@"
    const u32 header=getSerializer()->getHeaderSize(),stride=getSerializer()->getDataSize();
    const u32 count=getGalaxyNum();
    if(!pData||count>65535||header>dataSize||dataSize-header<2||stride==0||count>(dataSize-header-2)/stride) return -1;
    if(!validGalaxyHeader((u8*)getSerializer()->getBuffer(),header)) return -1;
    // Validate all output fields before writing any bytes.
    BinaryDataContentAccessor check((u8*)getSerializer()->getBuffer());
    std::vector<u8> probe(stride);
    u8* nameField=(u8*)check.getPointer("mGalaxyName",probe.data());
    if(!nameField||stride<2||nameField-probe.data()>stride-2)return -1;
    try{for(u32 i=0;i<count;++i)mSomeGalaxyStorages[i]->serialize(check,probe.data());}
    catch(const std::length_error&){return -1;}
"@
if(!$s.Contains($anchor)){throw 'serialize guard anchor missing'}
$s=$s.Replace($anchor,$guard)
Set-Content "$n\src\generated\GameDataGalaxyStorage.cpp" $s
