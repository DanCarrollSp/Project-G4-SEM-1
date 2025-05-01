#include "World.h"

World::World()
{
}

Image World::GenerateProceduralMap(int width, int height)
{
    //When called make the naviagtion grid the same size as the passed in map size params
    mapWidth = width;
    mapHeight = height;

    //CreateS an empty image of width * height, initially fill with color BLACK (empty/walkway)
    map = GenImageColor(width, height, BLACK);
    //Pointer to the pixel data of the image to alter pixels and their color
    pixels = LoadImageColors(map);

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            //Random walls
            if (rand() % 15 == 0) pixels[y * width + x] = WHITE;//Chances of a cell being a wall (random seeded in main.cpp)
            //Boarder walls
            if (y == 0 || y == height - 1 || x == 0 || x == width - 1) pixels[y * width + x] = WHITE;//Makes the map boarder all walls
            //Makes all center paths walkable
            if (x == 10 || y == 10) pixels[y * width + x] = BLACK;
            

            //make middle of each side of world border a door
            if (x == 10) if (y == 0 or y == height - 1) pixels[y * width + x] = BLUE;
            if (y == 10) if (x == 0 or x == height - 1) pixels[y * width + x] = BLUE;




            //Add bounding boxes for walls
            if (pixels[y * width + x].r == WHITE.r && pixels[y * width + x].g == WHITE.g && pixels[y * width + x].b == WHITE.b)
            {
                BoundingBox box;
                Vector3 min = { x, 0.0f, y };
                Vector3 max = { x + 1.0f, 1.0f, y + 1.0f };
                box.min = min;
                box.max = max;
                wallBoundingBoxes.push_back(box);
            }
            //Add bounding boxes for doors
            if (pixels[y * width + x].r == BLUE.r && pixels[y * width + x].g == BLUE.g && pixels[y * width + x].b == BLUE.b)
            {
                BoundingBox box;
                Vector3 min = { x, 0.0f, y };
                Vector3 max = { x + 1.0f, 1.0f, y + 1.0f };
                box.min = min;
                box.max = max;
                doorBoundingBoxes.push_back(box);
            }

        }
    }

    //Reapplys the modified pixel data to the image to set the worlds generation
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            ImageDrawPixel(&map, x, y, pixels[y * width + x]);
        }
    }

    return map;
}



std::vector<std::vector<bool>> World::CreateNavigationGrid() const
{
    //Creates a 2D grid the size of the map
    std::vector<std::vector<bool>> grid(mapHeight, std::vector<bool>(mapWidth, true));

    //Loop over image pixels to mark blocked vs walkable
    //Color Keys:
    //   WHITE = blocked
    //   BLUE  = walkable (door)
    //   BLACK = walkable
    //   else  = walkable
    for (int y = 0; y < mapHeight; y++)
    {
        for (int x = 0; x < mapWidth; x++)
        {
            bool isWalkable = true;//Default to walkable

            //White cells (walls)(blocked)
            if (pixels[y * mapWidth + x].r == WHITE.r && pixels[y * mapWidth + x].g == WHITE.g && pixels[y * mapWidth + x].b == WHITE.b)
            {
                isWalkable = false;
            }

            //Sets current cell of the nav grid to walkable or blocked
            grid[y][x] = isWalkable;
        }
    }

    return grid;
}



//Clears and then rebuilds the world collision boxes the image file
void World::BuildFromImage(const Image& img)
{
    //If we have an old pixel buffer, free it
    if (pixels)
    {
        UnloadImageColors(pixels);
        pixels = nullptr;
    }

    //Clear existing bounding boxes for walls and doors
    wallBoundingBoxes.clear();
    doorBoundingBoxes.clear();

    //If we had an old map image loaded, unload it
    if (map.data) UnloadImage(map);



    //Copy new image into our map
    map = ImageCopy(img);
    mapWidth = map.width;//Full image width (col)
    mapHeight = map.height;//Full image height (rows = layers * rowsPerLayer)

    //Load raw pixel colors into the array
    pixels = LoadImageColors(map);

    //Number of rows per single layer in the tall PNG
    int rowsPerLayer = MAP_LENGHT;

    //Loop over every row in the image
    for (int row = 0; row < mapHeight; ++row)
    {
        //Get which layer (vertical slice), and the z index within that layer
        int layer = row / rowsPerLayer;
        int z = row % rowsPerLayer;

        //Get the worldspace Y bounds for this layer
        float currentBottomLayer = static_cast<float>(layer);
        float CurrentTopLayer = currentBottomLayer + 1.0f;

        //For each column in the current row
        for (int x = 0; x < mapWidth; ++x)
        {
            //Fetch the pixel color at row x
            Color v = pixels[row * mapWidth + x];



            //Helper lambda to push a cube bounding box to that position
            auto push = [&](std::vector<BoundingBox>& vec)
                {
                    vec.push_back({ { static_cast<float>(x), currentBottomLayer, static_cast<float>(z) }, { static_cast<float>(x) + 1,   CurrentTopLayer, static_cast<float>(z) + 1 } });
                };

            //Helper to push four cube bounding boxes in a stair step order in a given direction
            auto push4StairSteps = [&](int direction, float baseY)
                {
                    float stepTopHeights[4] = { 0.25f, 0.50f, 0.75f, 1.0f };

                    for (int i = 0; i < 4; ++i)
                    {
                        BoundingBox box;

                        switch (direction)
                        {
                        case 0://N NORTH (step rise direction)
                            box.min = { static_cast<float>(x), baseY, z + i * 0.25f };
                            box.max = { x + 1.0f, baseY + stepTopHeights[i], z + (i + 1) * 0.25f };
                            break;
                        case 1: //S SOUTH
                            box.min = { static_cast<float>(x), baseY, z + (3 - i) * 0.25f };
                            box.max = { x + 1.0f, baseY + stepTopHeights[i], z + (4 - i) * 0.25f };
                            break;
                        case 2: //E EAST
                            box.min = { x + (3 - i) * 0.25f, baseY, static_cast<float>(z) };
                            box.max = { x + (4 - i) * 0.25f, baseY + stepTopHeights[i], z + 1.0f };
                            break;
                        case 3: //W WEST
                            box.min = { x + i * 0.25f, baseY, static_cast<float>(z) };
                            box.max = { x + (i + 1) * 0.25f, baseY + stepTopHeights[i], z + 1.0f };
                            break;
                        }
                        // Add this stair step as a wall (solid surface)
                        wallBoundingBoxes.push_back(box);
                    }
                };



            //Get block type by color
            bool wall = (v.r == WHITE.r && v.g == WHITE.g && v.b == WHITE.b);
            bool door = (v.r == BLUE.r && v.g == BLUE.g && v.b == BLUE.b);
            bool floor = (v.r == BROWN.r && v.g == BROWN.g && v.b == BROWN.b);
            bool stair = (v.r == ORANGE.r && v.g == ORANGE.g && v.b == ORANGE.b);

			//Place Default 1x1x1 bounding boxes for walls, doors, and floors
            if (wall)  push(wallBoundingBoxes);
            else if (door)  push(doorBoundingBoxes);
            else if (floor) push(wallBoundingBoxes); // floor is treated as solid surface
            //
            //Place 4 bounding boxes for stairs in 4 different directions N,S,E,W
            if (ColorEq(v, ORANGE_N)) push4StairSteps(0, currentBottomLayer);
            else if (ColorEq(v, ORANGE_S)) push4StairSteps(1, currentBottomLayer);
            else if (ColorEq(v, ORANGE_E)) push4StairSteps(2, currentBottomLayer);
            else if (ColorEq(v, ORANGE_W)) push4StairSteps(3, currentBottomLayer);
        }
    }
}



//Draws the minimap 40x40 around the player moving with them, clamping to the edge of the world and only showing tiles on the current layer
void World::DrawMiniMapSmall(const Player& player, const World& world, int screenW, int screenH)
{
    //Minimap size and position
    const int miniMapSize = 60;//Mini Map window size
	const int tileSize = 4;//Size of each tile in pixels within the window
    const int minimapOriginX = screenW - miniMapSize * tileSize - 32;//The X coordinate of the minimap origin
    const int minimapOriginY = 128;//The Y coordinate of the minimap origin



    //Get which layer (floor) the player is currently on
    const int levels = world.mapHeight / MAP_LENGHT;//Total number of vertical layers in the world
	int layer = (int)floorf(player.position.y);//Player current layer/floor index
    if (layer < 0 || layer >= levels) return;//Player is out of world bounds, nothing to draw



    //Get the center tile indexs based on player pos
    //floorf returns a floating point value that represents the largest integer that is less than or equal to x
    int cx = (int)floorf(player.position.x);//Players X tile coord
    int cz = (int)floorf(player.position.z);//Players Z (row) tile coord
    int half = miniMapSize / 2;//Half the window size in tiles

	//Clamp the 40×40 window to worlds max width and length
    int startX = std::max(0, cx - half);
    int startZ = std::max(0, cz - half);
    int endX = std::min(world.mapWidth, startX + miniMapSize);
    int endZ = std::min(MAP_LENGHT, startZ + miniMapSize);



    //Draws the minimap border background
    DrawRectangle( minimapOriginX - 1, minimapOriginY - 1, miniMapSize * tileSize + 2, miniMapSize * tileSize + 2, RED);

    //Iterate over each visible tile in the window
    for (int z = startZ; z < endZ; ++z)
    {
        for (int x = startX; x < endX; ++x)
        {
            //Put the index into the pixels for this layer
            int index = (layer * MAP_LENGHT + z) * MAP_WIDTH + x;
            Color src = world.pixels[index];//Color this tile

            //Determine the color - wall=WHITE, door=BLUE, empty=BLACK
            Color color = BLACK;//Empty
            if (src.r == WHITE.r && src.g == WHITE.g && src.b == WHITE.b) color = WHITE;//Wall
			else if (src.r == BLUE.r && src.g == BLUE.g && src.b == BLUE.b) color = BLUE;//Door
			else if (src.r == BROWN.r && src.g == BROWN.g && src.b == BROWN.b) color = BROWN;//Floor
			else if (ColorEq(src, ORANGE_N)) color = ORANGE_N;//Stairs N
            else if (ColorEq(src, ORANGE_S)) color = ORANGE_S;//Stairs S
            else if (ColorEq(src, ORANGE_E)) color = ORANGE_E;//Stairs E
            else if (ColorEq(src, ORANGE_W)) color = ORANGE_W;//Stairs W


            //Draws the tile on the minimap
            DrawRectangle(minimapOriginX + (x - startX) * tileSize, minimapOriginY + (z - startZ) * tileSize, tileSize, tileSize, color);
        }
    }

    //Draws the player marker as a green circle on top
    DrawCircle(minimapOriginX + (cx - startX) * tileSize + tileSize, minimapOriginY + (cz - startZ) * tileSize + tileSize, tileSize, GREEN);
}