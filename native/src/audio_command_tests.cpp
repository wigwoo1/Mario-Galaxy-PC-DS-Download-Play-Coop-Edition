#include <JSystem/JAudio2/JASCmdStack.hpp>
#include <cassert>
#include <cstdio>
struct Cmd:JASPortCmd{Cmd():JASPortCmd{JSULink<JASPortCmd>(this),nullptr,nullptr}{}};
static void count(JASPortArgs* args){++args->_04;}
int main(){JASPortArgs onceArgs{},stayArgs{};Cmd once,stay;assert(once.setPortCmd(count,&onceArgs));assert(stay.setPortCmd(count,&stayArgs));assert(once.addPortCmdOnce());assert(JASPortCmd::sCommandListStay.append(&stay));assert(!once.setPortCmd(count,&stayArgs));assert(!stay.setPortCmd(count,&onceArgs));JASPortCmd::execAllCommand();assert(onceArgs._04==1&&stayArgs._04==1&&!once.getSupervisor());JASPortCmd::execAllCommand();assert(onceArgs._04==1&&stayArgs._04==2);assert(once.setPortCmd(count,&onceArgs)&&once.addPortCmdOnce());JASPortCmd::execAllCommand();assert(onceArgs._04==2&&stayArgs._04==3);assert(JASPortCmd::sCommandListStay.remove(&stay));JASPortCmd::execAllCommand();assert(stayArgs._04==3);assert(!JASPortCmd::sCommandListOnce.getNumLinks()&&!JASPortCmd::sCommandListStay.getNumLinks());puts("PASS original audio one-shot/persistent command scheduling, queued configuration rejection and removal");}
