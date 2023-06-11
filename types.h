#ifndef TYPES_H_
#define TYPES_H_

#include <stdint.h>

#define SIGN(x) (x == 0 ? 0 : (x > 0 ? 1 : -1))

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;

#endif //TYPES_H_
