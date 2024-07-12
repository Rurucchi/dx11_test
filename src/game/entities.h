#include "../types.h"


struct Entity {
	float x;
	float y;
	float size;
} Entity;

struct Player {
	Entity entity;
	int life;
	float speed;
	int weapon;
	int armor;
} Player;