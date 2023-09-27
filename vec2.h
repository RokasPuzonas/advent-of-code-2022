#ifndef VEC2_H_
#define VEC2_H_

#include <stdbool.h>
#include "types.h"

typedef struct {
	i32 x, y;
} vec2;

#define VEC2(a, b) { .x = a, .y = b }

#define TYPEDEF_VEC2(type) typedef struct { type x, y; } vec2_##type

TYPEDEF_VEC2(u8);
TYPEDEF_VEC2(u32);

static bool vec2_eq(vec2 *A, i32 x, i32 y) {
	return A->x == x && A->y == y;
}

static bool vec2_eq2(vec2 A, vec2 B) {
	return A.x == B.x && A.y == B.y;
}

static bool vec2_u32_eq2(vec2_u32 A, vec2_u32 B) {
	return A.x == B.x && A.y == B.y;
}

#endif //VEC2_H_
