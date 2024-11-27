//Global Libs
#include "raylib.h"
#include <stdlib.h>
#include <time.h>

#define MAP_WIDTH 20
#define MAP_HEIGHT 20

class World
{
public:

	World();

	Image GenerateProceduralMap(int width, int height);

	Image map;
	Color* pixels;
};