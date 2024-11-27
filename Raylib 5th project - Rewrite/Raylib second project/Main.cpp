#include "Main.h"

int main(void)
{
    //Creates the fullscreen window
    InitWindow(screenWidth, screenHeight, "Rouge Like 2.5D Shooter");
    ToggleFullscreen();

    //Sets the mouse position to the center of the screen on start (stops game starting with you looking at the ceiling)
    SetMousePosition(screenWidth / 2, screenHeight / 2);

    //Sets up raylibs perspective camera
    camera.position = Vector3{ 10, 0.5f, 10 };//Camera pos
    camera.up = Vector3{ 0.0f, 10.0f, 0.0f };//Camera orientation
    camera.fovy = 45.0f;//Fov In degrees
    camera.projection = CAMERA_PERSPECTIVE;

    //Seed random time for map generation
    srand((unsigned int)time(NULL));
    //Generate random map
    Image imMap = GenerateProceduralMap(MAP_WIDTH, MAP_HEIGHT);
    miniMap = LoadTextureFromImage(imMap);//Minimap
    Mesh mesh = GenMeshCubicmap(imMap, Vector3{ 1.0f, 1.0f, 1.0f });//'3d' mesh
    model = LoadModelFromMesh(mesh);//Creates the 3d model based on the mesh
    Texture2D texture = LoadTexture("resources/cubicmap_atlas.png");//Loads textures for the map
    model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture;//Assigns textures to the mesh
    mapPixels = LoadImageColors(imMap);//Color array, converts 'image' data into map data for collisions (white = passavble, else = not passable)

    //Positions the map in the game world space
    mapPosition = { 0, 0, 0 };
    //Disables the cursor to lock the mouse to the screen
    DisableCursor();
    //
    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        //Updates
        Update();

        //Draws game
        Draw();
    }

    CloseWindow();
    return 0;
}

/// ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Game Logic run by update and draw

void Update()
{
    //Camera updates
    Vector3 oldCamPos = camera.position;//Save the cameras old position (to push the player back on collisions)
    UpdateCamera(&camera, CAMERA_FIRST_PERSON);//Updates the camera pos
    //Gets the cameras direction for collision
    Vector3 direction = Vector3Subtract(camera.target, camera.position);
    direction = Vector3Normalize(direction);


    //Player controls
    player.HandleInput();
    //Enemy AI (TODO)

    //Camera collision with mapPixels (white)
    bool collision = player.calcWallCollision(camera, mapPixels, mapPosition, MAP_WIDTH, MAP_HEIGHT, direction);
    if (collision)
    {
        camera.position = oldCamPos;
        camera.target = Vector3Add(oldCamPos, direction);
    }
}


void Draw()
{
    BeginDrawing();
    ClearBackground(LIGHTGRAY);//Clears screen
    //>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

    BeginMode3D(camera);//3d rendering
    DrawModel(model, mapPosition, 1.0f, WHITE);//Draws the model (walls) on white map tiles
    EndMode3D();

    //Draws the minimap
    DrawTextureEx(miniMap, Vector2{ screenWidth - MAP_WIDTH * 4.0f - 20, 20.0f }, 0.0f, 4.0f, WHITE);//Minimap
    DrawRectangleLines(screenWidth - MAP_WIDTH * 4 - 20, 20, MAP_WIDTH * 4, MAP_HEIGHT * 4, RED);//Minimap border
    //Draws the player
    player.Animate(screenWidth, screenHeight, camera, mapPosition);


    //>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    EndDrawing();
}