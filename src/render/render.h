/*  ----------------------------------- INFOS
	This header file contains data structures and game rendering related functions.
	
*/

#ifndef _RENDERH_
#define _RENDERH_

// custom libs
#include "../types.h"
#include "../platform/platform.h"
#include "../platform/file.h"

struct vertex
{
    f32 position[2];
    f32 uv[2];
    f32 color[3];
};

struct quad_mesh {
	i32 x;
	i32 y;
	i32 width;
	i32 height;
	// float orientation;
};

struct game_camera
{
    v2 position;
    f32 ratio;
    f32 scale; // height = scale, width = ratio * scale
};

// camera projection
mx game_OrthographicProjection(game_camera* camera, float width, float height)
{
    float L = camera->position.x - width / 2.f;
    float R = camera->position.x + width / 2.f;
    float T = camera->position.y + height / 2.f;
    float B = camera->position.y - height / 2.f;
    mx res = 
    {
        2.0f/(R-L),   0.0f,           0.0f,       0.0f,
        0.0f,         2.0f/(T-B),     0.0f,       0.0f,
        0.0f,         0.0f,           0.5f,       0.0f,
        (R+L)/(L-R),  (T+B)/(B-T),    0.5f,       1.0f,
    };
    
    return res;
}

#endif /* _RENDERH_ */