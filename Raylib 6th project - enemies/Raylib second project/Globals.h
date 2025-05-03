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

extern Globals globals;

//Stair colors
static const Color ORANGE_N = { 255,128, 0,255 };
static const Color ORANGE_S = { 255,160, 0,255 };
static const Color ORANGE_E = { 255,192, 0,255 };
static const Color ORANGE_W = { 255,224, 0,255 };

//Color equator
bool ColorEq(Color a, Color b);


//
void initWorldTextures();

//Block Textures
extern Texture2D TextureWall; extern Color WallColor;
extern Texture2D TextureDoor; extern Color DoorColor1; extern Color DoorColor2;
extern Texture2D TextureFloor; extern Color FloorColor;
extern Texture2D TextureStone; extern Color StoneColor;
extern Texture2D TextureDirt; extern Color DirtColor;
extern Texture2D TextureGrass; extern Color GrassColor;


//Brick
extern Texture2D TextureBrick1; extern Color Brick1Color;
extern Texture2D TextureBrick2; extern Color Brick2Color;
extern Texture2D TextureBrick3; extern Color Brick3Color;
extern Texture2D TextureBrick4; extern Color Brick4Color;

//Brick Face
extern Texture2D TextureBrickFace1; extern Color BrickFace1Color;
extern Texture2D TextureBrickFace2; extern Color BrickFace2Color;
extern Texture2D TextureBrickFace2Blood; extern Color BrickFace2BloodColor;
extern Texture2D TextureBrickFace3; extern Color BrickFace3Color;
extern Texture2D TextureBrickFace3Blood; extern Color BrickFace3BloodColor;
extern Texture2D TextureBrickFace4; extern Color BrickFace4Color;
extern Texture2D TextureBrickFace5; extern Color BrickFace5Color;
extern Texture2D TextureRedBrickTexture; extern Color RedBrickTextureColor;

//Cement
extern Texture2D TextureCementWall; extern Color CementWallColor;
extern Texture2D TextureCementWallBottom1; extern Color CementWallBottom1Color;
extern Texture2D TextureCementWallBottom2; extern Color CementWallBottom2Color;
extern Texture2D TextureCementWallBottom3; extern Color CementWallBottom3Color;
extern Texture2D TextureCementWallDeco1; extern Color CementWallDeco1Color;
extern Texture2D TextureCementWallDeco2; extern Color CementWallDeco2Color;
extern Texture2D TextureCementWallTop1; extern Color CementWallTop1Color;
extern Texture2D TextureCementWallTop2; extern Color CementWallTop2Color;

//Dark Steel
extern Texture2D TextureDarkSteel; extern Color DarkSteelColor;
extern Texture2D TextureDarkSteelBeam; extern Color DarkSteelBeamColor;
extern Texture2D TextureDarkSteelDeco; extern Color DarkSteelDecoColor;
extern Texture2D TextureDarkSteelHazard; extern Color DarkSteelHazardColor;
extern Texture2D TextureDarkSteelTop1; extern Color DarkSteelTop1Color;
extern Texture2D TextureDarkSteelTop2; extern Color DarkSteelTop2Color;
extern Texture2D TextureDarkSteelWall; extern Color DarkSteelWallColor;
extern Texture2D TextureDarkSteelWallDeco; extern Color DarkSteelWallDecoColor;

//Steel
extern Texture2D TextureOldSteel; extern Color OldSteelColor;
extern Texture2D TextureRustBeamUp; extern Color RustBeamUpColor;
extern Texture2D TextureRustBeamSide; extern Color RustBeamSideColor;
extern Texture2D TextureSteelBeam1; extern Color SteelBeam1Color;
extern Texture2D TextureSteelBeam2; extern Color SteelBeam2Color;
extern Texture2D TextureSteelDoor; extern Color SteelDoorColor;
extern Texture2D TextureSteelGrip1; extern Color SteelGrip1Color;
extern Texture2D TextureSteelGrip2; extern Color SteelGrip2Color;
extern Texture2D TextureSteelPlate; extern Color SteelPlateColor;
extern Texture2D TextureSteelReinforced; extern Color SteelReinforcedColor;

//Grates
extern Texture2D TextureGrate; extern Color GrateColor;
extern Texture2D TextureGrateRust; extern Color GrateRustColor;

//Spawns
extern Texture2D TextureSpawn; extern Color SpawnColor;


//Misc
extern Texture2D TEST_TEXTURE; extern Color TEST_COLOR;
extern Texture2D TextureSupportBeam; extern Color SupportBeamColor;
extern Texture2D TextureSwitchGreen; extern Color SwitchGreenColor;
extern Texture2D TextureSwitchRed; extern Color SwitchRedColor;
extern Texture2D TextureBigDoorLeft; extern Color BigDoorLeftColor1; extern Color BigDoorLeftColor2;
extern Texture2D TextureBigDoorRight; extern Color BigDoorRightColor1; extern Color BigDoorRightColor2;
extern Texture2D TextureFence; extern Color FenceColor;
extern Texture2D TextureVent; extern Color VentColor;