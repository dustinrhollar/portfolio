#ifndef APPCONFIG_H
#define APPCONFIG_H

#define global        static
#define local_persist static
#define internal      static

typedef int8_t  i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float  real32;
typedef double real64;

#define WIREFRAME 1

#define VOXEL 0

// Define which Procedural Terrain Generation algorithm to use
#define NOISE          1
#define THERMAL        0
#define HYDRAULIC      0
#define INVERSETHERMAL 0

// Define whether or not to write to file
// when doing proceural terrain gen
#define WRITE 1

// Comment to print Greyscale
// when doing proceural terrain gen
#define COLOR 0

#endif // APPCONFIG_H