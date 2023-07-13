#ifndef VEC2_H_
#define VEC2_H_

#include "types.h"

typedef struct {
	i32 x, y;
} vec2;

#define VEC2(a, b) { .x = a, .y = b }

#define TYPEDEF_VEC2(type) typedef struct { type x, y; } vec2_##type

TYPEDEF_VEC2(u8);
TYPEDEF_VEC2(u32);

#endif //VEC2_H_
