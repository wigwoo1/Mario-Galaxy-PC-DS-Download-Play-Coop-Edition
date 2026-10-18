#include <JSystem/JAudio2/JASBasicBank.hpp>
#include <cassert>
#include <cstring>
#include <new>
#include <cstdio>
int main(){alignas(JASBasicBank) unsigned char memory[sizeof(JASBasicBank)];std::memset(memory,0xa5,sizeof(memory));auto* bank=new(memory) JASBasicBank;for(int i=0;i<INST_NUM_MAX;++i)assert(bank->getInst(i)==nullptr);assert(bank->getInst(-1)==nullptr&&bank->getInst(INST_NUM_MAX)==nullptr);int token=0;auto* identity=reinterpret_cast<JASInst*>(&token);bank->setInst(INST_NUM_MAX-1,identity);assert(bank->getInst(INST_NUM_MAX-1)==identity&&bank->getInst(0)==nullptr);bank->~JASBasicBank();std::puts("PASS native original bank: all 240 pointer slots initialized, lookup bounds and last-slot identity");}
