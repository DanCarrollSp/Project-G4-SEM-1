#pragma once

//Global Libs
#include "raylib.h"
#include <rlgl.h>
#include <unordered_set>

const int MAP_WIDTH = 20;
const int MAP_HEIGHT = 20;

class Globals
{
public:

    Globals();


    //Draw textured cubes
    //Source: https://www.raylib.com/examples/models/loader.html?name=models_draw_cube_texture
    void DrawCubeTexture(Texture2D texture, Vector3 position, float width, float height, float length, Color color);
    
    //Draw textured cylinders
    void DrawTexturedCylinder(Texture2D texture, Vector3 position, float radius, float height, Color color);
    Mesh cylinderMesh;
    Model cylinderModel;
    std::unordered_set<int> assignedModelTextures;//Stored ids of textures that have been assigned to a model



    //Scale the minimap
    float miniMapScale = 8.0f;
};