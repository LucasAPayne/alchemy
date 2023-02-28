#pragma once

#include <glm/glm.hpp>

#include <stdint.h>

#define ARRAY_COUNT(array) (sizeof((array)) / sizeof((array)[0]))

#define internal        static
#define local_persist   static
#define global_variable static

typedef uint32_t b32;
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
