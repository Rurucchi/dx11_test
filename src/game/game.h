/*  ----------------------------------- INFOS
	This header file is the base to most of the game elements and logic
	
	
*/
#ifndef _GAMEH_
#define _GAMEH_

// LOCAL DEPENDENCIES : types.h
#include "../types.h"
#include "../render/dx11.h"
#include "../render/render.h"
#include "../render/atlas.h"


// game objects (entities)
enum entity_type {approach_circle, hit_circle, slider_ball, slider_tick, slider_body, spinner};

struct game_entity {
	f32 aliveTime;
	f32 maxAliveTime;
	entity_type type;
	i16 x;
	i16 y;
	ui16 size;
	ui8 order;
};

// game options & gamemode

struct game_map
{
	f32 OD;
	f32 CS;
	f32 AR;
    f32 HP;
	// Map specific data
	float duration;
	// game_map_elements *elements;
	ui16 elementCount;
};

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

// game logic

void game_update_entity(game_entity* loadedEntities, game_entity** renderList, ui32 entityCount, ui32* renderCount, f64 elapsed){
	for(int i = 0; i < entityCount; i++) {
		loadedEntities[i].aliveTime -= elapsed;
		if(loadedEntities[i].aliveTime > 0){
			switch(loadedEntities[i].type) {
				case approach_circle: {
					// resize approach circle relative to alive time
					float size = lerp(100, 400, loadedEntities[i].aliveTime / loadedEntities[i].maxAliveTime);
					loadedEntities[i].size = (ui16)size;
					
					// add it to the render queue
					renderList[*renderCount] = &loadedEntities[i];
					(*renderCount)++;
					break;
				};
				
				case hit_circle: {
					// add circle (and number, its existence implied by hitcircle order) to the render queue
					renderList[*renderCount] = &loadedEntities[i];
					(*renderCount)++;
					break;
				};
			};
		};
	};
};

// game rendering

void game_resizeCamera(game_camera* camera, HWND window) {
	RECT rect;
	GetClientRect(window, &rect);
	
	int height = rect.bottom - rect.top;
	int width = rect.right - rect.left;
	camera->ratio = width/height;
	camera->scale = height;
};

// circle is a pointer to a hit_circle array here
void render_entity(game_entity** renderList, ui32 renderCount, f64 elapsed, render_context* rContext) {
	// check if there are any entity to draw
	if(renderCount != 0) {
		
		// render each entity
		for(int i = 0; i < renderCount; i++) {
			// entity will be considered "dead" if its aliveTime is zero or less and will not be rendered.
			if(renderList[i]->aliveTime > 0) {
				
				// handle its type
				switch(renderList[i]->type) {
					case approach_circle: {
						quad_mesh approachCircleQuad = {
							.x = renderList[i]->x,
							.y = renderList[i]->y,
							.width = renderList[i]->size,
							.height = renderList[i]->size,
						};
						
						rtpAtlasSprite approachCircle_pos = rtpDescAtlas[2];
						v2 texposMin = texture_convertTexposMinToNDC(approachCircle_pos);
						v2 texposMax = texture_convertTexposMaxToNDC(approachCircle_pos);
						render_queue_quad(&approachCircleQuad, texposMin, texposMax, rContext);
						break;
					}
					
					case hit_circle: {
						quad_mesh hitCircleQuad = {
						.x = renderList[i]->x,
						.y = renderList[i]->y,
						.width = renderList[i]->size,
						.height = renderList[i]->size,
						};
						
						quad_mesh numberQuad = {
							.x = renderList[i]->x,
							.y = renderList[i]->y,
							.width = renderList[i]->size / 2,
							.height = renderList[i]->size / 2,
						};
						
						rtpAtlasSprite hitCircle_pos = rtpDescAtlas[0];
						v2 texposMin = texture_convertTexposMinToNDC(hitCircle_pos);
						v2 texposMax = texture_convertTexposMaxToNDC(hitCircle_pos);
						
						render_queue_quad(&hitCircleQuad, texposMin, texposMax, rContext);
						
						rtpAtlasSprite number_pos = rtpDescAtlas[4];
						texposMin = texture_convertTexposMinToNDC(number_pos);
						texposMax = texture_convertTexposMaxToNDC(number_pos);
						
						render_queue_quad(&numberQuad, texposMin, texposMax, rContext);
						break;
					}
					
					default: {
						break;
					}
				};
			};
		};
	};
};


#endif /* _GAMEH_ */