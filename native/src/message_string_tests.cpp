#include "native_tagged_message.hpp"
#include <Game/Util/StringUtil.hpp>
#include <cassert>
#include <cstdio>
int main(){NativeBmgText::Token text;text.code='A';NativeBmgText::Token tag;tag.code=0x1a;tag.tag={6,2,0,1};NativeTaggedMessage skip({text,tag,text});assert(!MR::isMessageEditorNextTag(skip.data()+1));assert(MR::getStringLengthWithMessageTag(skip.data())==5);tag.tag={6,1,0,1};NativeTaggedMessage next({text,tag,text});assert(MR::isMessageEditorNextTag(next.data()+1));assert(MR::getStringLengthWithMessageTag(next.data())==1);tag.tag={6,1,1,0};NativeTaggedMessage swapped({text,tag,text});assert(!MR::isMessageEditorNextTag(swapped.data()+1));assert(MR::getStringLengthWithMessageTag(swapped.data())==5);std::puts("PASS original message traversal: preserved packed tags, BE next ID, stop and skip lengths");}
