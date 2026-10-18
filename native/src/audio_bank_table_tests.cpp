#include <JSystem/JAudio2/JAUBankTable.hpp>
#include <JSystem/JAudio2/JASBankTable.hpp>
#include <cassert>
#include <cstdio>
#include <cstring>
#include <new>
int main(){alignas(JASBankTable<256>) unsigned char storage[sizeof(JASBankTable<256>)];std::memset(storage,0xa5,sizeof(storage));auto* table=new(storage) JASBankTable<256>;for(unsigned i=0;i<256;++i)assert(!table->getBank(i));auto* token=reinterpret_cast<JASBank*>(std::uintptr_t(0x1234567887654321ull));table->registBank(255,token);assert(table->getBank(255)==token&&!table->getBank(256));JASBank* slots[3];JAUBankTable dynamic(17,slots,3);assert(!dynamic.getBank(0)&&!dynamic.getBank(2)&&!dynamic.getBank(3));dynamic.registBank(2,token);assert(dynamic.getBank(2)==token&&dynamic.field_0xc==17);JAUWaveBankTable waves;for(unsigned i=0;i<255;++i)assert(!waves.getWaveBank(i));auto* wave=reinterpret_cast<JASWaveBank*>(std::uintptr_t(0x8877665544332211ull));waves.registWaveBank(254,wave);assert(waves.getWaveBank(254)==wave&&!waves.getWaveBank(255));puts("PASS original native bank table initialization, last slots, lookup bounds and full-width identities.");}
