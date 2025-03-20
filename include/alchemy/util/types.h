#pragma once

#include <cglm/struct.h>

#include <float.h>
#include <stdint.h>

#define ALCHEMY_EXPORT __declspec(dllexport)

#define countof(array) (sizeof((array)) / sizeof((array)[0]))
#define lengthof(array) (countof(array) - 1)

#if ALCHEMY_DEBUG
    #define ASSERT(expression) if(!(expression)) {*(int *)0 = 0;}
#else
    #define ASSERT(...)
#endif

#define INVALID_CODE_PATH() ASSERT(!"Invalid Code Path")
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

typedef vec2s v2;
typedef ivec2s iv2;
typedef vec3s v3;
typedef ivec3s iv3;
typedef vec4s v4;
typedef ivec4s iv4;

typedef mat2s m2;
typedef mat3s m3;
typedef mat4s m4;
