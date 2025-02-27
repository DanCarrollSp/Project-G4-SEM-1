#pragma once

//Global Libs
#include "raylib.h"
#include <stdlib.h>
#include <time.h>

//Local Libs
#include "Globals.h"
#include <vector>

class World
{
public:

	World();

	//Map creation
	Image GenerateProceduralMap(int width, int height);//Creates random map
	std::vector<std::vector<bool>> CreateNavigationGrid() const;//Creates path finding grid based on the generated map

	//Accesors
	std::vector<BoundingBox> GetWallBoundingBoxes() const { return wallBoundingBoxes; }
	std::vector<BoundingBox> GetDoorBoundingBoxes() const { return doorBoundingBoxes; }

	//Storage
	Image map;
	Color* pixels;
	std::vector<BoundingBox> wallBoundingBoxes;
	std::vector<BoundingBox> doorBoundingBoxes;

	int mapWidth;
	int mapHeight;
};