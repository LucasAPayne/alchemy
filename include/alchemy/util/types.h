#pragma once

#include <cglm/struct.h>

#include <float.h>
#include <stdint.h>

#ifdef ALCHEMY_NO_HOT_RELOAD
    #define ALCHEMY_EXPORT
#else
    #define ALCHEMY_EXPORT __declspec(dllexport)
#endif

#define countof(array) (sizeof((array)) / sizeof((array)[0]))
#define lengthof(array) (countof(array) - 1)

#define u64_high_low(hi, lo) ((u64)(hi) << 32) | (lo)

#define INVALID_CODE_PATH() ASSERT(0, "Invalid Code Path")
#define INVALID_DEFAULT_CASE() default: {INVALID_CODE_PATH();} break

#define internal static
#define persist  static
#define global   static

#define true  1
#define false 0

#define F32_MIN FLT_MIN
#define F32_MAX FLT_MAX

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef ptrdiff_t size;
typedef size_t usize;

typedef int8_t  i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef int32_t  b32;

typedef float f32;
typedef double f64;

typedef unsigned char ubyte;

#define v2(x, y) (v2){(x), (y)}
typedef vec2s v2;
#define iv2(x, y) (iv2){(x), (y)}
typedef ivec2s iv2;

#define v3(x, y, z) (v3){(x), (y), (z)}
typedef vec3s v3;
#define iv3(x, y, z) (iv3){(x), (y), (z)}
typedef ivec3s iv3;

#define v4(x, y, z, w) (v4){(x), (y), (z), (w)}
typedef vec4s v4;
#define iv4(x, y, z, w) (iv4){(x), (y), (z), (w)}
typedef ivec4s iv4;

typedef mat2s m2;
typedef mat3s m3;
typedef mat4s m4;
