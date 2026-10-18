#pragma once
// Pinned MSL cmath DEG_TO_RAD: retain its float pi and float divisor.
template<class T> constexpr auto nativeDegreesToRadians(T degrees){return degrees*(3.14159265358979323846f/180.0f);}
#ifndef DEG_TO_RAD
#define DEG_TO_RAD(degrees) nativeDegreesToRadians((degrees))
#endif
