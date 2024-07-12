/*  ----------------------------------- INFOS
    This header file contains most of the file and data handling related.
    
*/

// LOCAL DEPENDENCIES : types.h

#ifndef _FILEH_
#define _FILEH_

#include <windows.h>
#include "../types.h"

#define internal static
#define local_persist static
#define global_variable static


// LOCAL DEPENDENCIES : TYPES.H

//  ------------------------------------ STRUCTS

typedef struct complete_file {
	i32 size;
	void *memory;
} completeFile;

// disable compiler memory alligment
#pragma pack(push, 1)
typedef struct file_bitmap_header
{
    // header: 14 bytes
    char formatID[2];
    ui32 size;
    ui32 pad;
    ui32 offset; // start of the data array
} file_bitmap_header;

typedef struct file_bitmap_info_header
{
    ui32 headerSize;
    i32 width;
    i32 height;
    ui16 numPlanes;
    ui16 bpp; // bits per pixel
    char compression[4];
    ui32 imageSize;
    i32 hres;
    i32 vres;
    ui32 numColors; // number of colors in the palette
    ui32 numImpColors; // number of important colors used
} file_bitmap_info_header;
#pragma pack(pop)

// texture
typedef struct texture {
	void *memory;
	int memorySize;
	int width;
	int height;
	int bytesPerPixel;
} texture;

//  ------------------------------------ FILE RELATED FUNCTIONS

void FILE_FULLREAD(char *Location, complete_file *file){
	HANDLE rawFile = CreateFileA(Location, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	
	//check the handle
    if (rawFile == INVALID_HANDLE_VALUE)
    {
		OutputDebugStringA("FILE OPENING ERROR\n");
    } else {
		LARGE_INTEGER fileSize;
	
		GetFileSizeEx(rawFile,
		  &fileSize
		);
	
		ui32 fileSize32 = SafeTruncateUInt64(fileSize.QuadPart);
		file->memory = VirtualAlloc(0, fileSize32, MEM_COMMIT, PAGE_READWRITE);
		ReadFile(rawFile, file->memory, fileSize32, NULL, NULL);
		file->size = fileSize32;
	}

}

void FILE_FULLFREE(complete_file *file){
	// free the memory of the file
	VirtualFree(
	  file->memory,
	  file->size,
	  MEM_RELEASE
	);
}

#endif /* _FILEH_ */