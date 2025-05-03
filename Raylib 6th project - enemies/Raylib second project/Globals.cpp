#include "Globals.h"

Globals::Globals()
{
}



// Draw cube textured
// NOTE: Cube position is the center position
void Globals::DrawCubeTexture(Texture2D texture, Vector3 position, float width, float height, float length, Color color)
{
    float x = position.x;
    float y = position.y;
    float z = position.z;

    // Set desired texture to be enabled while drawing following vertex data
    rlSetTexture(texture.id);

    // Vertex data transformation can be defined with the commented lines,
    // but in this example we calculate the transformed vertex data directly when calling rlVertex3f()
    //rlPushMatrix();
        // NOTE: Transformation is applied in inverse order (scale -> rotate -> translate)
        //rlTranslatef(2.0f, 0.0f, 0.0f);
        //rlRotatef(45, 0, 1, 0);
        //rlScalef(2.0f, 2.0f, 2.0f);

    rlBegin(RL_QUADS);
    rlColor4ub(color.r, color.g, color.b, color.a);
    // Front Face
    rlNormal3f(0.0f, 0.0f, 1.0f);       // Normal Pointing Towards Viewer
    rlTexCoord2f(0.0f, 1.0f); rlVertex3f(x - width / 2, y - height / 2, z + length / 2);  // Bottom Left Of The Texture and Quad
    rlTexCoord2f(1.0f, 1.0f); rlVertex3f(x + width / 2, y - height / 2, z + length / 2);  // Bottom Right Of The Texture and Quad
    rlTexCoord2f(1.0f, 0.0f); rlVertex3f(x + width / 2, y + height / 2, z + length / 2);  // Top Right Of The Texture and Quad
    rlTexCoord2f(0.0f, 0.0f); rlVertex3f(x - width / 2, y + height / 2, z + length / 2);  // Top Left Of The Texture and Quad
    // Back Face
    rlNormal3f(0.0f, 0.0f, -1.0f);     // Normal Pointing Away From Viewer
    rlTexCoord2f(1.0f, 1.0f); rlVertex3f(x - width / 2, y - height / 2, z - length / 2);  // Bottom Right Of The Texture and Quad
    rlTexCoord2f(1.0f, 0.0f); rlVertex3f(x - width / 2, y + height / 2, z - length / 2);  // Top Right Of The Texture and Quad
    rlTexCoord2f(0.0f, 0.0f); rlVertex3f(x + width / 2, y + height / 2, z - length / 2);  // Top Left Of The Texture and Quad
    rlTexCoord2f(0.0f, 1.0f); rlVertex3f(x + width / 2, y - height / 2, z - length / 2);  // Bottom Left Of The Texture and Quad
    // Top Face
    rlNormal3f(0.0f, 1.0f, 0.0f);       // Normal Pointing Up
    rlTexCoord2f(0.0f, 0.0f); rlVertex3f(x - width / 2, y + height / 2, z - length / 2);  // Top Left Of The Texture and Quad
    rlTexCoord2f(0.0f, 1.0f); rlVertex3f(x - width / 2, y + height / 2, z + length / 2);  // Bottom Left Of The Texture and Quad
    rlTexCoord2f(1.0f, 1.0f); rlVertex3f(x + width / 2, y + height / 2, z + length / 2);  // Bottom Right Of The Texture and Quad
    rlTexCoord2f(1.0f, 0.0f); rlVertex3f(x + width / 2, y + height / 2, z - length / 2);  // Top Right Of The Texture and Quad
    // Bottom Face
    rlNormal3f(0.0f, -1.0f, 0.0f);     // Normal Pointing Down
    rlTexCoord2f(1.0f, 0.0f); rlVertex3f(x - width / 2, y - height / 2, z - length / 2);  // Top Right Of The Texture and Quad
    rlTexCoord2f(0.0f, 0.0f); rlVertex3f(x + width / 2, y - height / 2, z - length / 2);  // Top Left Of The Texture and Quad
    rlTexCoord2f(0.0f, 1.0f); rlVertex3f(x + width / 2, y - height / 2, z + length / 2);  // Bottom Left Of The Texture and Quad
    rlTexCoord2f(1.0f, 1.0f); rlVertex3f(x - width / 2, y - height / 2, z + length / 2);  // Bottom Right Of The Texture and Quad
    // Right face
    rlNormal3f(1.0f, 0.0f, 0.0f);       // Normal Pointing Right
    rlTexCoord2f(1.0f, 1.0f); rlVertex3f(x + width / 2, y - height / 2, z - length / 2);  // Bottom Right Of The Texture and Quad
    rlTexCoord2f(1.0f, 0.0f); rlVertex3f(x + width / 2, y + height / 2, z - length / 2);  // Top Right Of The Texture and Quad
    rlTexCoord2f(0.0f, 0.0f); rlVertex3f(x + width / 2, y + height / 2, z + length / 2);  // Top Left Of The Texture and Quad
    rlTexCoord2f(0.0f, 1.0f); rlVertex3f(x + width / 2, y - height / 2, z + length / 2);  // Bottom Left Of The Texture and Quad
    // Left Face
    rlNormal3f(-1.0f, 0.0f, 0.0f);    // Normal Pointing Left
    rlTexCoord2f(0.0f, 1.0f); rlVertex3f(x - width / 2, y - height / 2, z - length / 2);  // Bottom Left Of The Texture and Quad
    rlTexCoord2f(1.0f, 1.0f); rlVertex3f(x - width / 2, y - height / 2, z + length / 2);  // Bottom Right Of The Texture and Quad
    rlTexCoord2f(1.0f, 0.0f); rlVertex3f(x - width / 2, y + height / 2, z + length / 2);  // Top Right Of The Texture and Quad
    rlTexCoord2f(0.0f, 0.0f); rlVertex3f(x - width / 2, y + height / 2, z - length / 2);  // Top Left Of The Texture and Quad
    rlEnd();
    //rlPopMatrix();

    rlSetTexture(0);
}




void Globals::DrawTexturedCylinder(Texture2D texture, Vector3 position, float radius, float height, Color color)
{
    cylinderMesh = GenMeshCylinder(radius, height, 30);
    cylinderModel = LoadModelFromMesh(cylinderMesh);

    //Apply the texture to the model
    cylinderModel.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture;

    //Draw the textured cylinder
    DrawModel(cylinderModel, position, 1.0f, color);
    UnloadModel(cylinderModel);
}




void Globals::DrawStair(const Vector3& base, int dir, Texture2D tex)
{
	//Heights of each of the individual cubes making up the steps of the stairs
    const float stepHeights[4] = { 0.25f,0.50f,0.75f,1.00f };

    for (int i = 0; i < 4; ++i)
    {
		//Vars for the position and size of the smaller cubes
        Vector3 pos = base;
        Vector3 size = { 1, stepHeights[i], 0.25f };


        //Pos and size of each of the steps
        switch (dir)
        {
        case 0: pos.z += (i - 1.5f) * 0.25f; break; // N North
        case 1: pos.z += (1.5f - i) * 0.25f; break; // S South
        case 2: pos.x += (1.5f - i) * 0.25f;  size = { 0.25f,stepHeights[i],1 }; break; // E East
        case 3: pos.x += (i - 1.5f) * 0.25f;  size = { 0.25f,stepHeights[i],1 }; break; // W West
        }

        //Now shift it up by half of the height so that the bottom of each step is actually on the floor
        pos.y = floorf(base.y) + stepHeights[i] * 0.5f;

        //Draw the steps/cubes
        DrawCubeTexture(tex, pos, size.x, size.y, size.z, WHITE);
    }
}



//Returns if two colors are equal
bool ColorEq(Color a, Color b)
{
    return a.r == b.r && a.g == b.g && a.b == b.b;
}






Globals globals;

//Block Textures
Texture2D TextureWall; Color WallColor;
Texture2D TextureDoor; Color DoorColor1; Color DoorColor2;
Texture2D TextureFloor; Color FloorColor;
Texture2D TextureStone; Color StoneColor;
Texture2D TextureDirt; Color DirtColor;
Texture2D TextureGrass; Color GrassColor;


//Brick
Texture2D TextureBrick1; Color Brick1Color;
Texture2D TextureBrick2; Color Brick2Color;
Texture2D TextureBrick3; Color Brick3Color;
Texture2D TextureBrick4; Color Brick4Color;

//Brick Face
Texture2D TextureBrickFace1; Color BrickFace1Color;
Texture2D TextureBrickFace2; Color BrickFace2Color;
Texture2D TextureBrickFace2Blood; Color BrickFace2BloodColor;
Texture2D TextureBrickFace3; Color BrickFace3Color;
Texture2D TextureBrickFace3Blood; Color BrickFace3BloodColor;
Texture2D TextureBrickFace4; Color BrickFace4Color;
Texture2D TextureBrickFace5; Color BrickFace5Color;
Texture2D TextureRedBrickTexture; Color RedBrickTextureColor;

//Concrete
Texture2D TextureCementWall; Color CementWallColor;
Texture2D TextureCementWallBottom1; Color CementWallBottom1Color;
Texture2D TextureCementWallBottom2; Color CementWallBottom2Color;
Texture2D TextureCementWallBottom3; Color CementWallBottom3Color;
Texture2D TextureCementWallDeco1; Color CementWallDeco1Color;
Texture2D TextureCementWallDeco2; Color CementWallDeco2Color;
Texture2D TextureCementWallTop1; Color CementWallTop1Color;
Texture2D TextureCementWallTop2; Color CementWallTop2Color;

//Dark Steel
Texture2D TextureDarkSteel; Color DarkSteelColor;
Texture2D TextureDarkSteelBeam; Color DarkSteelBeamColor;
Texture2D TextureDarkSteelDeco; Color DarkSteelDecoColor;
Texture2D TextureDarkSteelHazard; Color DarkSteelHazardColor;
Texture2D TextureDarkSteelTop1; Color DarkSteelTop1Color;
Texture2D TextureDarkSteelTop2; Color DarkSteelTop2Color;
Texture2D TextureDarkSteelWall; Color DarkSteelWallColor;
Texture2D TextureDarkSteelWallDeco; Color DarkSteelWallDecoColor;

//Steel
Texture2D TextureOldSteel; Color OldSteelColor;
Texture2D TextureRustBeamUp; Color RustBeamUpColor;
Texture2D TextureRustBeamSide; Color RustBeamSideColor;
Texture2D TextureSteelBeam1; Color SteelBeam1Color;
Texture2D TextureSteelBeam2; Color SteelBeam2Color;
Texture2D TextureSteelDoor; Color SteelDoorColor;
Texture2D TextureSteelGrip1; Color SteelGrip1Color;
Texture2D TextureSteelGrip2; Color SteelGrip2Color;
Texture2D TextureSteelPlate; Color SteelPlateColor;
Texture2D TextureSteelReinforced; Color SteelReinforcedColor;

//Grates  
Texture2D TextureGrate; Color GrateColor;
Texture2D TextureGrateRust; Color GrateRustColor;

//Spawns
Texture2D TextureSpawn; Color SpawnColor;

//Misc
Texture2D TEST_TEXTURE; Color TEST_COLOR;
Texture2D TextureSupportBeam; Color SupportBeamColor;
Texture2D TextureSwitchGreen; Color SwitchGreenColor;
Texture2D TextureSwitchRed; Color SwitchRedColor;
Texture2D TextureBigDoorLeft; Color BigDoorLeftColor1; Color BigDoorLeftColor2;
Texture2D TextureBigDoorRight; Color BigDoorRightColor1; Color BigDoorRightColor2;
Texture2D TextureFence; Color FenceColor;
Texture2D TextureVent; Color VentColor;



void initWorldTextures()
{
    //Default Textures
    TextureWall = LoadTexture("resources/World/Wall.png"); WallColor = { 255,255,255,255 };//White
    TextureDoor = LoadTexture("resources/World/Door.png"); DoorColor1 = { 0,0,255,255 }; DoorColor2 = { 0,0,252,255 };//Blues
    TextureFloor = LoadTexture("resources/World/Floor.png"); FloorColor = { 128,128,128,255 };//Grey
    TextureStone = LoadTexture("resources/World/Stone.png"); StoneColor = { 127,128,128,255 };//Grey
    TextureDirt = LoadTexture("resources/World/Dirt.png");  DirtColor = { 51,25,0,255 };//Brown
    TextureGrass = LoadTexture("resources/World/Grass.png"); GrassColor = { 0,255,0,255 };//Green


    //Brick
    TextureBrick1 = LoadTexture("resources/World/Brick1.png"); Brick1Color = { 102,51,0,255 };//Light Brown
    TextureBrick2 = LoadTexture("resources/World/Brick2.png"); Brick2Color = { 102,52,0,255 };//Light Brown
    TextureBrick3 = LoadTexture("resources/World/Brick3.png"); Brick3Color = { 102,53,0,255 };//Light Brown
    TextureBrick4 = LoadTexture("resources/World/Brick4.png"); Brick4Color = { 102,54,0,255 };//Light Brown

    //Brick Face
    TextureBrickFace1 = LoadTexture("resources/World/BrickFace1.png"); BrickFace1Color = { 102,55,0,255 };//Light Brown
    TextureBrickFace2 = LoadTexture("resources/World/BrickFace2.png"); BrickFace2Color = { 102,56,0,255 };//Light Brown
    TextureBrickFace2Blood = LoadTexture("resources/World/BrickFace2Blood.png"); BrickFace2BloodColor = { 102,57,0,255 };//Light Brown
    TextureBrickFace3 = LoadTexture("resources/World/BrickFace3.png"); BrickFace3Color = { 102,58,0,255 };//Light Brown
    TextureBrickFace3Blood = LoadTexture("resources/World/BrickFace3Blood.png"); BrickFace3BloodColor = { 102,59,0,255 };//Light Brown
    TextureBrickFace4 = LoadTexture("resources/World/BrickFace4.png"); BrickFace4Color = { 102,60,0,255 };//Light Brown
    TextureBrickFace5 = LoadTexture("resources/World/BrickFace5.png"); BrickFace5Color = { 102,61,0,255 };//Light Brown
    TextureRedBrickTexture = LoadTexture("resources/World/RedBrickTexture.png"); RedBrickTextureColor = { 102,62,0,255 };//Light Brown

    //Cement
    TextureCementWall = LoadTexture("resources/World/CementWall.png"); CementWallColor = { 102,63,0,255 };//Light Brown
    TextureCementWallBottom1 = LoadTexture("resources/World/CementWallBottom1.png"); CementWallBottom1Color = { 102,64,0,255 };//Light Brown
    TextureCementWallBottom2 = LoadTexture("resources/World/CementWallBottom2.png"); CementWallBottom2Color = { 102,65,0,255 };//Light Brown
    TextureCementWallBottom3 = LoadTexture("resources/World/CementWallBottom3.png"); CementWallBottom3Color = { 102,66,0,255 };//Light Brown
    TextureCementWallDeco1 = LoadTexture("resources/World/CementWallDeco1.png"); CementWallDeco1Color = { 102,67,0,255 };//Light Brown
    TextureCementWallDeco2 = LoadTexture("resources/World/CementWallDeco2.png"); CementWallDeco2Color = { 102,68,0,255 };//Light Brown
    TextureCementWallTop1 = LoadTexture("resources/World/CementWallTop1.png"); CementWallTop1Color = { 102,69,0,255 };//Light Brown
    TextureCementWallTop2 = LoadTexture("resources/World/CementWallTop2.png"); CementWallTop2Color = { 102,70,0,255 };//Light Brown


    //Dark Steel
    TextureDarkSteel = LoadTexture("resources/World/DarkSteel.png"); DarkSteelColor = { 126,128,128,255 };//Grey
    TextureDarkSteelBeam = LoadTexture("resources/World/DarkSteelBeam.png"); DarkSteelBeamColor = { 125,128,128,255 };//Grey
    TextureDarkSteelDeco = LoadTexture("resources/World/DarkSteelDeco.png"); DarkSteelDecoColor = { 124,128,128,255 };//Grey
    TextureDarkSteelHazard = LoadTexture("resources/World/DarkSteelHazard.png"); DarkSteelHazardColor = { 123,128,128,255 };//Grey
    TextureDarkSteelTop1 = LoadTexture("resources/World/DarkSteelTop1.png"); DarkSteelTop1Color = { 122,128,128,255 };//Grey
    TextureDarkSteelTop2 = LoadTexture("resources/World/DarkSteelTop2.png"); DarkSteelTop2Color = { 121,128,128,255 };//Grey
    TextureDarkSteelWall = LoadTexture("resources/World/DarkSteelWall.png"); DarkSteelWallColor = { 120,128,128,255 };//Grey
    TextureDarkSteelWallDeco = LoadTexture("resources/World/DarkSteelWallDeco.png"); DarkSteelWallDecoColor = { 119,128,128,255 };//Grey

    //Steel
    TextureOldSteel = LoadTexture("resources/World/OldSteel.png"); OldSteelColor = { 118,128,128,255 };//Grey
    TextureRustBeamUp = LoadTexture("resources/World/RustBeamUp.png"); RustBeamUpColor = { 117,128,128,255 };//Grey
    TextureRustBeamSide = LoadTexture("resources/World/RustBeamSide.png"); RustBeamSideColor = { 116,128,128,255 };//Grey
    TextureSteelBeam1 = LoadTexture("resources/World/SteelBeam1.png"); SteelBeam1Color = { 115,128,128,255 };//Grey
    TextureSteelBeam2 = LoadTexture("resources/World/SteelBeam2.png"); SteelBeam2Color = { 114,128,128,255 };//Grey
    TextureSteelDoor = LoadTexture("resources/World/SteelDoor.png"); SteelDoorColor = { 113,128,128,255 };//Grey
    TextureSteelGrip1 = LoadTexture("resources/World/SteelGrip1.png"); SteelGrip1Color = { 112,128,128,255 };//Grey
    TextureSteelGrip2 = LoadTexture("resources/World/SteelGrip2.png"); SteelGrip2Color = { 111,128,128,255 };//Grey
    TextureSteelPlate = LoadTexture("resources/World/SteelPlate.png"); SteelPlateColor = { 110,128,128,255 };//Grey
    TextureSteelReinforced = LoadTexture("resources/World/SteelReinforced.png"); SteelReinforcedColor = { 109,128,128,255 };//Grey

    //Grates
    TextureGrate = LoadTexture("resources/World/Grate.png"); GrateColor = { 129,128,128,255 };//Grey
    TextureGrateRust = LoadTexture("resources/World/GrateRust.png"); GrateRustColor = { 130,128,128,255 };//Grey


    //Spawns
    TextureSpawn = LoadTexture("resources/World/Spawn.png"); SpawnColor = { 0,254,0,255 };//Green


    //Misc
    TEST_TEXTURE = LoadTexture("resources/World/TEST.png"); TEST_COLOR = { 255,51,255,255 };//Pink
    TextureSupportBeam = LoadTexture("resources/World/SupportBeam.png"); SupportBeamColor = { 131,128,128,255 };//Grey
    TextureSwitchGreen = LoadTexture("resources/World/SwitchGreen.png"); SwitchGreenColor = { 255,255,0,255 };//Yellow
    TextureSwitchRed = LoadTexture("resources/World/SwitchRed.png"); SwitchRedColor = { 254,255,0,255 };//Yellow
    TextureBigDoorLeft = LoadTexture("resources/World/BigDoorLeft.png"); BigDoorLeftColor1 = { 0,0,254,255 }; BigDoorLeftColor2 = { 0,0,251,255 };//Blue
    TextureBigDoorRight = LoadTexture("resources/World/BigDoorRight.png"); BigDoorRightColor1 = { 0,0,253,255 }; BigDoorRightColor2 = { 0,0,250,255 };//Blue
    TextureFence = LoadTexture("resources/World/Fence.png"); FenceColor = { 132,128,128,255 };//Grey
    TextureVent = LoadTexture("resources/World/Vent.png"); VentColor = { 133,128,128,255 };//Grey
}