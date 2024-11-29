#include "World.h"

World::World()
{
}

Image World::GenerateProceduralMap(int width, int height)
{
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