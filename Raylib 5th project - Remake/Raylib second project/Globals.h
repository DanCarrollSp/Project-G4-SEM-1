#pragma once

//Global Libs
#include "raylib.h"
#include <rlgl.h>

const int MAP_WIDTH = 20;
const int MAP_HEIGHT = 20;

class Globals
{
public:

    Globals();

    //Source: https://www.raylib.com/examples/models/loader.html?name=models_draw_cube_texture
    void DrawCubeTexture(Texture2D texture, Vector3 position, float width, float height, float length, Color color);

    //Scale the minimap
    float miniMapScale = 8.0f;
};