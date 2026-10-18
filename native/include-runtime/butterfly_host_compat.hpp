#pragma once

#ifndef DEG_TO_RAD_MULT_CONSTANT
#define DEG_TO_RAD_MULT_CONSTANT (3.14159265358979323846f / 180.0f)
#endif
#ifndef DEG_TO_RAD
#define DEG_TO_RAD(value) ((value) * DEG_TO_RAD_MULT_CONSTANT)
#endif
