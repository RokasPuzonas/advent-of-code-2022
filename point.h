#ifndef POINT_H_
#define POINT_H_

#include "types.h"

typedef struct {
	int x, y;
} Point;

typedef struct {
	u8 x, y;
} Point_u8;

typedef struct {
	u32 x, y;
} Point_u32;

#endif //POINT_H_
