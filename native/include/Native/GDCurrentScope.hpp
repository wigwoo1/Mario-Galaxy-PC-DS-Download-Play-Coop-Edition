#pragma once
#include <revolution/gd/GDBase.h>
namespace Native {
class GDCurrentScope {
    GDLObj* previous;
public:
    GDCurrentScope():previous(__GDCurrentDL){}
    ~GDCurrentScope(){GDSetCurrent(previous);}
    GDCurrentScope(const GDCurrentScope&)=delete;
    GDCurrentScope& operator=(const GDCurrentScope&)=delete;
};
}
