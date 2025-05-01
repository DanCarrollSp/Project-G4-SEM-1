#pragma once

//Global Libs
#include "raylib.h"
#include <rlgl.h>
#include <unordered_set>

const int MAP_WIDTH = 100;
const int MAP_LENGHT = 100;
const int MAP_LEVELS = 32;

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

    //Draw textured stairs
    void DrawStair(const Vector3& base, int dir, Texture2D tex);


    //Scale the minimap
    float miniMapScale = 10.0f;
};

//Stair colors
static const Color ORANGE_N = { 255,128, 0,255 };
static const Color ORANGE_S = { 255,160, 0,255 };
static const Color ORANGE_E = { 255,192, 0,255 };
static const Color ORANGE_W = { 255,224, 0,255 };

//Color equator
bool ColorEq(Color a, Color b);