/*  ----------------------------------- INFOS
	This header file is the base to most of the other files and headers in this project.
	
*/

#ifndef _TYPESH_
#define _TYPESH_

#define internal static
#define local_persist static
#define global_variable static
#define RAYMATH_IMPLEMENTATION

#include <windows.h>
#include "raymath.h"

//  ----------------------------------- GENERAL

typedef unsigned int uint;

typedef uint8_t ui8;
typedef uint16_t ui16;
typedef uint32_t ui32;
typedef uint64_t ui64;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef float f32;
typedef double f64;

typedef Vector2 v2;
typedef Vector3 v3;
typedef Vector4 v4;
typedef Matrix mx;


//  ----------------------------------- type checkings and conversions

#define Assert(expression)                                                     \
  if(!(expression)) {                                                          \
    *(int *)0 = 0;                                                             \
}

ui32 SafeTruncateUInt64(ui64 value){
  Assert(value <= 0xFFFFFFFF);
  ui32 result = (ui32)value;
  return result;
}

//  ----------------------------------- PLATFORM


// typedef struct file_bitmap {
	// file_bitmap_header *bitmapHeader;
	// file_bitmap_info_header *bitmapInfoHeader;
// } file_bitmap;

// ----------------------------------- GAME LOGIC



//  ----------------------------------- RENDERING



#endif /* _TYPESH_ */