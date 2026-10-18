#pragma once
#include "Game/Util/StringUtil.hpp"
#include <stdexcept>
#include <iostream>
inline void nativeStringTests(){
    auto check=[](bool ok){if(!ok)throw std::runtime_error("native identifier string regression");};
    check(MR::isEqualString("DamageWait","DamageWait"));check(!MR::isEqualString("DamageWait","damagewait"));check(MR::isEqualStringCase("DamageWait","damageWAIT"));check(MR::_stricmp("","abc")<0&&MR::_stricmp("abc","")>0&&MR::_stricmp("abc","abcd")<0&&MR::_stricmp("ABCD","abc")>0);
    check(MR::isEqualString("壁押し","壁押し")&&!MR::isEqualString("壁押し","壁右歩き"));check(MR::isEqualStringCase("壁押しA","壁押しa"));
    for(unsigned byte=1;byte<256;++byte){char a[2]={static_cast<char>(byte),0};char b[2]={static_cast<char>(byte>='A'&&byte<='Z'?byte+32:byte),0};check(MR::_stricmp(a,b)==0);}
    char highA[2]={static_cast<char>(0x80),0},highB[2]={static_cast<char>(0xff),0};check(MR::_stricmp(highA,highB)<0);
    std::cout<<"PASS: animation identifier comparisons, ASCII case folding and UTF-8 byte preservation\n";
}
