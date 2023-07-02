#ifndef TYPES_H_
#define TYPES_H_

#include <stdint.h>

#define SIGN(x) (x == 0 ? 0 : (x > 0 ? 1 : -1))
#define ARRAY_LEN(x) (sizeof(x)/sizeof(x[0]))

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

#endif //TYPES_H_
