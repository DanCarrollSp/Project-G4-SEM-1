#pragma once

//Global Libs
#include "raylib.h"
#include "raymath.h"
#include "time.h"
#include <random>

//Local Libs
#include "Globals.h"
#include "Scenes.h"
#include "Player.h"
#include "World.h"
#include "Enemy.h"

#include "ParticleSystem.h"

#include "UI.h"


//Functions
void Update();
void Draw();

void particles();

void debugControls();
void debug();


//Resolution
const int screenWidth = 1920;
const int screenHeight = 1080;
//Shaders
Shader alphaShader;

//Game Setup
Camera camera = { 0 };//Game camera
Vector3 mapPosition;//Maps world position
Texture2D miniMap;//Minimap cubicmap
//
std::vector<std::vector<bool>> navGrid;//Grid for path finding

Texture2D floorTexture;
Texture2D ceilingTexture;
Texture2D wallTexture;
Texture2D doorTexture;
Texture2D bloodTexture;
Color* mapPixels;//Color array for collisions

Texture2D barrelTexture;

//Particles
float particleSize;
ParticleParams bloodParams;

//Disable mouse once
bool mouseDisabled = false;
//Debugging
Color crosshairColor;
bool paused = false;
bool debugMode = false;
bool stopParticles = true;
bool stopEnemy = false;
bool enemyMove = true;

//Objects
Globals globals;
Scenes scenes;
Player player;
World world;
Enemy enemy;

ParticleSystem particleSystem;

UI gameUI;