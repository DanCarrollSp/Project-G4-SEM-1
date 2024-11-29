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
    Image imMap = world.GenerateProceduralMap(MAP_WIDTH, MAP_HEIGHT);
    miniMap = LoadTextureFromImage(imMap);//Minimap
    
    //Texture assignments
    floorTexture = LoadTexture("resources/floorTexture.png");
    ceilingTexture = LoadTexture("resources/ceilingTexture.png");
    wallTexture = LoadTexture("resources/wallTexture.png");
    doorTexture = LoadTexture("resources/door.png");

    //Map creation using walls, doors, etc
    mapPixels = LoadImageColors(imMap);//Color map, converts 'image' pixel color data into map data for collisions (black = passavble, else = not passable)


    //Position of the map in the game world space
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

    for (int y = 0; y < MAP_HEIGHT; y++)
    {
        for (int x = 0; x < MAP_WIDTH; x++)
        {
            //Get the pixel color from the map
            int index = y * MAP_WIDTH + x;
            Color pixelColor = mapPixels[index];

            //Recenter  to match up walls+doors to collisions
            Vector3 obsticalPosition = { x + 0.5f, 0.5f, y + 0.5f };
            //Floor and ceiling offseted positions
            Vector3 floorPosition = { x + 0.5f, -0.5f, y + 0.5f };
            Vector3 ceilingPosition = { x + 0.5f, 1.5f, y + 0.5f };


            //Draw floor under BLACK pixels
            if (pixelColor.r == BLACK.r && pixelColor.g == BLACK.g && pixelColor.b == BLACK.b)
                globals.DrawCubeTexture(floorTexture, floorPosition, 1.0f, 1.0f, 1.0f, WHITE);

            //Draw ceiling over BLACK pixels
            if (pixelColor.r == BLACK.r && pixelColor.g == BLACK.g && pixelColor.b == BLACK.b)
                globals.DrawCubeTexture(ceilingTexture, ceilingPosition, 1.0f, 1.0f, 1.0f, WHITE);

            //Draw wall on WHITE pixels
            if (pixelColor.r == WHITE.r && pixelColor.g == WHITE.g && pixelColor.b == WHITE.b)
                globals.DrawCubeTexture(wallTexture, obsticalPosition, 1.0f, 1.0f, 1.0f, WHITE);

            //Draw door on BLUE pixels
            if (pixelColor.r == BLUE.r && pixelColor.g == BLUE.g && pixelColor.b == BLUE.b)
                globals.DrawCubeTexture(doorTexture, obsticalPosition, 1.0f, 1.0f, 1.0f, WHITE);
        }
    }

    EndMode3D();


    //Draws the minimap
    float scale = globals.miniMapScale;
    DrawTextureEx(miniMap, Vector2{ screenWidth - MAP_WIDTH * scale - MAP_WIDTH, MAP_WIDTH }, 0.0f, scale, WHITE);//Minimap
    DrawRectangleLines(screenWidth - MAP_WIDTH * scale - MAP_WIDTH, MAP_WIDTH, MAP_WIDTH * scale, MAP_HEIGHT * scale, RED);//Minimap border
    //Draws the player
    player.Animate(screenWidth, screenHeight, camera, mapPosition);


    //>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    EndDrawing();
}