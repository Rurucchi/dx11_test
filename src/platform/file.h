/*  ----------------------------------- INFOS
    This header file contains most of the file and data handling related.
    
*/

// LOCAL DEPENDENCIES : types.h

#ifndef _FILEH_
#define _FILEH_

#include <windows.h>
#include <wincodec.h>
#include "../types.h"

#define internal static
#define local_persist static
#define global_variable static


// LOCAL DEPENDENCIES : TYPES.H

//  ------------------------------------ STRUCTS

typedef struct complete_file {
	ui32 size;
	void *memory;
} completeFile;

typedef struct complete_img {
	ui32 x;
	ui32 y;
	ui32 channels_in_file;
	void *memory;
} complete_img;

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
} texture;

//  ------------------------------------ FILE RELATED FUNCTIONS

HANDLE file_createHandle(char *location) {
	HANDLE rawFile = CreateFileA(location, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	
	if (rawFile == INVALID_HANDLE_VALUE)
    {
		OutputDebugStringA("FILE OPENING ERROR\n");
	}
	
	return rawFile;
}

void file_fullread(char *location, complete_file *file){
	HANDLE rawFile = file_createHandle(location);
	
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
		file->memory = VirtualAlloc(0, fileSize32, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
		ReadFile(rawFile, file->memory, fileSize32, NULL, NULL);
		file->size = fileSize32;
	}

}

void file_fullfree(complete_file *file){
	// free the memory of the file
	VirtualFree(
	  file->memory,
	  file->size,
	  MEM_RELEASE
	);
}


// todo: finish this
complete_img file_decodePNG(char *location, complete_file *file) {
	complete_img img = {0};
	
	file_fullread(location, file);
	int x; 
	int y;
	int channels_in_file;
	img.memory = (void*)stbi_load_from_memory((stbi_uc*)file->memory, file->size, &x, &y, &channels_in_file, 4);
	img.x = (ui32)x;
	img.y = (ui32)y;
	img.channels_in_file = (ui32)channels_in_file;
	return img;
}

#endif /* _FILEH_ */