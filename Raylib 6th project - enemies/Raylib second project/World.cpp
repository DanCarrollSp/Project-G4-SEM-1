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

    //Loop over image pixels to mark blocked vs. walkable
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
