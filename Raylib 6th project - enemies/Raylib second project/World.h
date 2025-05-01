#pragma once

//Global Libs
#include "raylib.h"
#include <stdlib.h>
#include <time.h>
#include <vector>
//Local Libs
#include "Player.h"
#include "Globals.h"


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


	//Replaces the current map with image then rebuild
	void BuildFromImage(const Image& img);

	static void DrawMiniMapSmall(const Player& pl, const World& w, int screenW, int screenH);
};