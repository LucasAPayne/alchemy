#pragma once

#pragma warning(push)
#pragma warning(disable: 4996)
#include <cglm/struct.h>
#pragma warning(pop)

#include <stdbool.h>
#include <stdint.h>

#define ARRAY_COUNT(array) (sizeof((array)) / sizeof((array)[0]))

#if ALCHEMY_DEBUG
    #define ASSERT(expression) if(!(expression)) {*(int *)0 = 0;}
#endif

#define internal        static
#define local_persist   static
#define global_variable static

typedef bool b8;
typedef int32_t b32;
typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef size_t usize;

typedef int8_t  i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

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