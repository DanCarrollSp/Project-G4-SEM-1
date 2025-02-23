#pragma once

//Global Libs
#include "raylib.h"
#include <stdlib.h>
#include <time.h>

//Local Libs
#include "Globals.h"

class World
{
public:

	World();

	Image GenerateProceduralMap(int width, int height);

	Image map;
	Color* pixels;
};