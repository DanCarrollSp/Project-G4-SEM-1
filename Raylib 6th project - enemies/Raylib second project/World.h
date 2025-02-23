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
	std::vector<BoundingBox> GetWallBoundingBoxes() const { return wallBoundingBoxes; }
	std::vector<BoundingBox> GetDoorBoundingBoxes() const { return doorBoundingBoxes; }

	Image map;
	Color* pixels;
	std::vector<BoundingBox> wallBoundingBoxes;
	std::vector<BoundingBox> doorBoundingBoxes;
};