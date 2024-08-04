/*  ----------------------------------- INFOS
	This header file is the base to most of the game elements and logic
	
	
*/
#ifndef _GAMEH_
#define _GAMEH_

// LOCAL DEPENDENCIES : types.h
#include "../types.h"

// todo: make mods working, add support for other mods
struct mods {
	// difficulty reduction
	ui8 easy;
	ui8 nofail;
	ui8 halftime;
	// difficulty increase
	ui8 hardRock;
	ui8 suddenDeath;
	ui8 perfect;
	ui8 doubleTime;
	ui8 nightcore;
	ui8 hidden;
	ui8 flashlight;
};

struct score {
	ui32 perfect;
	ui32 good;
	ui32 meh;
	ui32 miss;
};

struct game_camera
{
    v2 position;
    f32 ratio;
    f32 scale; // height = scale, width = ratio * scale
};

void game_resizeCamera(game_camera* camera, HWND window) {
	RECT rect;
	GetClientRect(window, &rect);
	
	int height = rect.bottom - rect.top;
	int width = rect.right - rect.left;
	camera->ratio = width/height;
	camera->scale = height;
};

#endif /* _GAMEH_ */