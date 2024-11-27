//Global Libs
#include "raylib.h"
#include "raymath.h"
#include "time.h"
#include <random>

//Local Libs
#include "Player.h"
#include "World.h"

//Map scale
#define MAP_WIDTH 20
#define MAP_HEIGHT 20


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
Model model;//3d model for the mesh
Color* mapPixels;//Color array for collisions

//Objects
Player player;