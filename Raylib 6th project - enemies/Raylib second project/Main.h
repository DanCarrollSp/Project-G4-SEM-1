#pragma once

//Global Libs
#include "raylib.h"
#include "raymath.h"
#include "time.h"
#include <random>
#include <cfloat>
#include "rlgl.h"
//Local Libs
#include "Globals.h"
#include "Scenes.h"
#include "Player.h"
#include "Shooting.h"
#include "World.h"
#include "Enemy.h"
#include "EntitySpawner.h"
#include "ParticleSystem.h"
#include "ParticleEngine.h"
#include "WorldEditor.h"
#include "Decal.h"
#include "UI.h"


//Functions
void Update();
void Draw();
//
void particles();
//
void shooting();
//
void debugControls();
void debug();
//
void testLevel();



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



///Textures
void InitTextures();
//Tiles
Texture2D floorTexture;
Texture2D ceilingTexture;
Texture2D wallTexture;
Texture2D doorTexture;
//Particles
Texture2D bloodTexture;
//Decals
Texture2D shellCasing;
Texture2D bulletHole;
//Misc
Texture2D barrelTexture;



//Color array for collisions
Color* mapPixels;



//Particles
float particleSize;
ParticleParams bloodParams;
ParticleParams shellCasingParams;

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
static std::vector<Enemy> enemies;
EntitySpawner spawner(enemies);

ParticleSystem particleSystem;
ParticleEngine particleEngine;

WorldEditor worldEditor;

DecalManager decalManager;

UI gameUI;

//Shooting
Vector3 crosshair;
BulletHitResult hitResult;
Vector3 enemyCollision;

//Init map maker once
bool mapMakerInit = false;

//Helper for release builds
bool chechVec3(const Vector3& vec1, const Vector3& vec2);
