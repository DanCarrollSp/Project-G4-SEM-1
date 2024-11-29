#pragma once

//Global Libs
#include "raylib.h"
#include "raymath.h"
#include "time.h"
#include <random>

//Local Libs
#include "Globals.h"
#include "Player.h"
#include "World.h"


//Functions
void Update();
void Draw();

//Resolution
const int screenWidth = 1920;
const int screenHeight = 1080;

//Game Setup
Camera camera = { 0 };//Game camera
Vector3 mapPosition;//Maps world position
Texture2D miniMap;//Minimap cubicmap

Texture2D floorTexture;
Texture2D ceilingTexture;
Texture2D wallTexture;
Texture2D doorTexture;
Color* mapPixels;//Color array for collisions

Texture2D barrelTexture;

//Objects
Globals globals;
Player player;
World world;