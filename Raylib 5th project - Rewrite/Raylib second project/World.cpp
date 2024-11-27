#include "World.h"

Image GenerateProceduralMap(int width, int height)
{
    //CreateS an empty image of width * height, initially fill with color BLACK (empty/walkway)
    Image map = GenImageColor(width, height, BLACK);
    Color* pixels = LoadImageColors(map);//Pointer to the pixel data of the image to alter pixels and their color

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            if (rand() % 6 == 0) pixels[y * width + x] = WHITE;//Chances of a cell being a wall (random seeded in main.cpp)
            else pixels[y * width + x] = BLACK;//Else its a walkway

            if (y == 0 || y == height - 1 || x == 0 || x == width - 1) pixels[y * width + x] = WHITE;//Makes the boarder all walls
            if (y == 10 && x == 10) pixels[y * width + x] = BLACK;//Makes sure the spawn point is a walkable area and not a wall
        }
    }

    //leave gaps to make future doors into different rooms
    //pixels[1 * width + 1] = BLACK;
    //pixels[(height - 2) * width + (width - 2)] = BLACK;

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
