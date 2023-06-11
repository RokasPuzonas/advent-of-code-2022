#ifndef VEC2_H_
#define VEC2_H_

#include "types.h"

typedef struct {
	int x, y;
} vec2;

#define TYPEDEF_VEC2(type) typedef struct { type x, y; } vec2_##type

TYPEDEF_VEC2(u8);
TYPEDEF_VEC2(u32);

#endif //VEC2_H_
