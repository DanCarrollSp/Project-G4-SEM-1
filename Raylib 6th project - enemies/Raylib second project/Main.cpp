#include "Main.h"

Vector3 enemyCollision;
bool isPaused = false;

bool debbuging = false;

bool enemyUpdate = false;

int main(void)
{
    //Creates the fullscreen window
    InitWindow(screenWidth, screenHeight, "Rouge Like 2.5D Shooter");
    ToggleFullscreen();

    //Sets the mouse position to the center of the screen on start (stops game starting with you looking at the ceiling)
    SetMousePosition(screenWidth / 2, screenHeight / 2);

    //Sets up raylibs perspective camera
    camera.position = Vector3{ 10, 0.6f, 10 };//Camera pos
    camera.up = Vector3{ 0.0f, 10.0f, 0.0f };//Camera orientation
    camera.fovy = 45.0f;//Fov In degrees
    camera.projection = CAMERA_PERSPECTIVE;

    //Seed random time for map generation
    srand((unsigned int)time(NULL));
    //Generate random map
    Image imMap = world.GenerateProceduralMap(MAP_WIDTH, MAP_HEIGHT);
    miniMap = LoadTextureFromImage(imMap);//Minimap
    
    //Generate nav gird
    navGrid = world.CreateNavigationGrid();
    
    //Texture assignments
    floorTexture = LoadTexture("resources/floorTexture.png");
    ceilingTexture = LoadTexture("resources/ceilingTexture.png");
    wallTexture = LoadTexture("resources/wallTexture.png");
    doorTexture = LoadTexture("resources/door.png");
    //
    barrelTexture = LoadTexture("resources/barrel.png");
    //
    bloodTexture = LoadTexture("resources/blood.png");

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

        if (IsKeyPressed(KEY_SPACE))
        {
            isPaused = !isPaused;
        }

        if (IsKeyPressed(KEY_F1))
        {
            debbuging = !debbuging;
        }

        if (IsKeyPressed(KEY_F2))
        {
            enemyUpdate = !enemyUpdate;
        }

        
        //Updates
        if (!isPaused) Update();


		ParticleParams bloodParams;
		bloodParams.position = enemyCollision;
        bloodParams.spawnCount = 9;
        bloodParams.minSpeed = 0.30f;
        bloodParams.maxSpeed = 0.70f;
        bloodParams.startColor = RED;
        bloodParams.endColor = MAROON;
        bloodParams.minLifetime = 0.5f;
        bloodParams.maxLifetime = 0.80f;
        bloodParams.gravity = { 0.1f, -0.1f, 0.1 };
        bloodParams.startSize = 0.1f;
        bloodParams.endSize = 0.3f;

        bloodParams.texture = &bloodTexture;

        particleSystem.Instantiate(bloodParams);
        if (!isPaused) particleSystem.UpdateAll(GetFrameTime());


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
    /// Movement & collision detection
    //Track oldpos for collisions
    Vector3 oldCamPos = camera.position;
    //Movement
    UpdateCamera(&camera, CAMERA_FIRST_PERSON);//Update the camera (movement)
    //Collision
    player.PreventBoundingBoxCollisions(world.GetWallBoundingBoxes(), player.hitbox, camera, oldCamPos);//Prevent player from walking through bounding boxes
    player.PreventBoundingBoxCollision(enemy.GetBoundingBox(), player.hitbox, camera, oldCamPos);//Prevent player from walking through bounding boxes


    //Player controls
	player.update(camera);
    player.HandleInput();
	player.closeToWallCheck(camera, world.GetWallBoundingBoxes());
    //Enemy AI ()
    enemy.Update();
    if (enemyUpdate) enemy.Move(player.position, navGrid, world.GetWallBoundingBoxes(), GetFrameTime());
}


void Draw()
{
    BeginDrawing();
    ClearBackground(BLACK);//Clears screen
    //>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    BeginMode3D(camera);//Start of 3D Rendering


	//World drawing (walls, doors, floor, ceiling)
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


	/// Draw entites ///

    debug();//Debugging visuals ( example - shows box collider outlines and raycasts)
    
    enemy.Draw(camera);//Draws the enemy, camera for billboarding
    particleSystem.DrawAll(camera);
    
    

    EndMode3D(); //End of 3D rendering
	//Start of drawing

    //Draws the minimap
    float scale = globals.miniMapScale / 2;
    DrawTextureEx(miniMap, Vector2{ screenWidth - MAP_WIDTH * scale - MAP_WIDTH, MAP_WIDTH }, 0.0f, scale, WHITE);//Minimap
    DrawRectangleLines(screenWidth - MAP_WIDTH * scale - MAP_WIDTH, MAP_WIDTH, MAP_WIDTH * scale, MAP_HEIGHT * scale, RED);//Minimap border

    //Draws the players hand and updates its animations
    player.Animate(screenWidth, screenHeight, camera, mapPosition);





    //>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
	EndDrawing(); //End of drawing
}


void debug()
{
    if (debbuging)
    {
        //>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
        ///  Bounding Boxes

        //Wall bounding boxes
    {
        std::vector<BoundingBox> boundingBoxes = world.GetWallBoundingBoxes();
        for (const auto& box : boundingBoxes) DrawBoundingBox(box, RED); // Draw bounding box in red
    }
    //Door bounding boxes
    {
        std::vector<BoundingBox> boundingBoxes = world.GetDoorBoundingBoxes();
        for (const auto& box : boundingBoxes) DrawBoundingBox(box, GREEN); // Draw bounding box in red
    }
    //Player bounding box
    DrawBoundingBox(player.hitbox, RED);


    //>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    /// Raycast hit points

    //Wall-bullet collision point
    for (int i = 0; i < world.GetWallBoundingBoxes().size(); i++)
    {
        Vector3 wallCollision = player.calcBulletCollision(camera, world.GetWallBoundingBoxes()[i]);
        if (wallCollision != Vector3{ 0, 0, 0 }) DrawSphere(wallCollision, 0.1f, BLUE);
    }
    //Door-bullet collision point
    for (int i = 0; i < world.GetDoorBoundingBoxes().size(); i++)
    {
        Vector3 doorCollision = player.calcBulletCollision(camera, world.GetDoorBoundingBoxes()[i]);
        if (doorCollision != Vector3{ 0, 0, 0 }) DrawSphere(doorCollision, 0.1f, GREEN);
    }


    //Draws a RED sphere on a shot walls (drawn over by blue hit detection for now)
    if (player.hitTarget && player.shot) DrawSphere(player.hitPoint, 0.1f, RED);
}

	//Draws a circle crosshair on the first piece of a raycasted line to represent the crosshair
    Vector3 rayDirection = Vector3Normalize(Vector3Subtract(camera.target, camera.position));
    Vector3 rayLinePoint = Vector3Add(camera.position, Vector3Scale(rayDirection, 0.1f));
    DrawSphere(rayLinePoint, 0.0002f, crosshairColor);

    //Draws a BLUE sphere on enemy hitboxes and makes the crosshair red while aiming at enemies
	enemyCollision = player.calcBulletCollision(camera, enemy.hitbox);//returns 0,0,0 if no collision, otherwise returns the collision point
    if (enemyCollision != Vector3{ 0, 0, 0 })
    {
        DrawSphere(enemyCollision, 0.1f, BLUE);
        crosshairColor = RED;
    }
    else crosshairColor = WHITE;
}























/*
{
    Vector3 pos1 = { 9.75,0, 10};
    Vector3 pos2 = { 10.25,0, 10 };
    Vector3 pos3 = { 9.75,0, 9.5 };



    globals.DrawTexturedCylinder(barrelTexture, pos3, 0.117, 0.35, WHITE);

    globals.DrawTexturedCylinder(barrelTexture, pos1, 0.117, 0.35, WHITE);

    globals.DrawTexturedCylinder(floorTexture, pos2, 0.117, 0.35, WHITE);
}
*/