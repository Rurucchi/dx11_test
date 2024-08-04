#include "../types.h"

struct approach_circle {
	ui16 x;
	ui16 y;
	ui8 size;
};

struct hit_circle {
	ui16 x;
	ui16 y;
	ui16 order;
};

struct slider {
	ui16 x;
	ui16 y;
	ui16 order;
	ui8 sliderbreak;;
};

struct slider_tick {
	float x;
	float y;
	float size;
};

struct spinner {
	int duration;
}