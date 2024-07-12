#include "../types.h"

struct game_camera
{
    v2 position;
    f32 ratio;
    f32 scale; // height = scale, width = ratio * scale
};