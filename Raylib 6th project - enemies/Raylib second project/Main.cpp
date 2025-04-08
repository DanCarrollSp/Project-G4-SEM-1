#include "Main.h"
#include "rlgl.h"
#include "Scenes.h"

Vector3 enemyCollision;


int main(void)
{
    //Creates the fullscreen window
    InitWindow(screenWidth, screenHeight, "Rouge Like 2.5D Shooter");
    ToggleFullscreen();

    //Sets the mouse position to the center of the screen on start (stops game starting with you looking at the ceiling)
    SetMousePosition(screenWidth / 2, screenHeight / 2);

    //Sets up raylibs perspective camera
    camera.position = Vector3{ 10, 0.6f, 10 };//Camera pos
    camera.up = Vector3{ 0.0f, 1.0f, 0.0f };//Camera orientation
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
    //SetTextureFilter(bloodTexture, TEXTURE_FILTER_POINT);
    //SetTextureFilter(texture, TEXTURE_FILTER_BILINEAR);

    //Map creation using walls, doors, etc
    mapPixels = LoadImageColors(imMap);//Color map, converts 'image' pixel color data into map data for collisions (black = passavble, else = not passable)

    //Shaders
    alphaShader = LoadShader(NULL, "shaders/alpha.fs");
	//Particle setup
	particles();

    InitAudioDevice();



    //Position of the map in the game world space
    mapPosition = { 0, 0, 0 };
    //
    SetTargetFPS(60);


    while (!WindowShouldClose())
    {

		//Main menu >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
        if (scenes.sceneMainMenu)
        {
            scenes.Update();
            scenes.Draw(camera);
        }

		//Gameplay >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
        if (scenes.sceneGameplay)
        {
            //Disables the cursor to lock the mouse to the screen (once)
            if (mouseDisabled == false)
            {
                DisableCursor();
                mouseDisabled = true;

                gameUI.LoadAssets("resources/fonts/Blood.ttf", "resources/fonts/Ammo.otf", "resources/images/ak_icon.png");
            }


			//Allows debug controls while paused (including the ability to pause and unpause)
            debugControls();

            if (!paused)
            {
                //Updates
                Update();
            }
            //Draws game
            Draw();
        }
    }

    if(!WindowShouldClose) CloseWindow();
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
    if (!stopEnemy)player.PreventBoundingBoxCollision(enemy.GetBoundingBox(), player.hitbox, camera, oldCamPos);//Prevent player from walking through bounding boxes


    //Player controls
    player.update(camera);
    player.HandleInput();
    player.closeToWallCheck(camera, world.GetWallBoundingBoxes());
    if (crosshairColor.r == RED.r && player.justFired)
    {
        particles();
        particleSystem.Instantiate(bloodParams);//Shooting
    }
    //Enemy AI ()
    if (!stopEnemy)enemy.Update();
    if (!enemyMove)enemy.Move(player.position, navGrid, world.GetWallBoundingBoxes(), GetFrameTime());
	//Particles
    particleSystem.UpdateAll(GetFrameTime());
}


void Draw()
{
    BeginDrawing();
    ClearBackground(BLACK);//Clears screen
    //>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    BeginMode3D(camera);//Start of 3D Rendering
    //BeginBlendMode(BLEND_ALPHA);

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


	/// Draw entites /////>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    BeginShaderMode(alphaShader);
    debug();//Debugging visuals ( example - shows box collider outlines and raycasts)
    
    if(!stopEnemy)enemy.Draw(camera);//Draws the enemy, camera for billboarding
	particleSystem.DrawAll(camera);//Draws all active particle effects

    EndShaderMode();
    EndMode3D();
	//Start of drawing//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


    //Draws the minimap
    float scale = globals.miniMapScale / 2;
    DrawTextureEx(miniMap, Vector2{ screenWidth - MAP_WIDTH * scale - MAP_WIDTH, MAP_WIDTH + 100 }, 0.0f, scale, WHITE);//Minimap
    DrawRectangleLines(screenWidth - MAP_WIDTH * scale - MAP_WIDTH, MAP_WIDTH + 100, MAP_WIDTH * scale, MAP_HEIGHT * scale, RED);//Minimap border

    //Draws the players hand and updates its animations
    player.Animate(screenWidth, screenHeight, camera, mapPosition);

    //UI
    //auto [currentAmmo, maxAmmo] = player.GetAmmo();
    auto ammo = player.GetAmmo();
    auto currentAmmo = std::get<0>(ammo);
    auto maxAmmo = std::get<1>(ammo);
    gameUI.Draw(100, currentAmmo, maxAmmo);
	gameUI.DrawStatic();

	if (paused) DrawTexture(gameUI.pauseTexture, 0 , 0 , WHITE);//Pause menu
    
    //>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
	EndDrawing(); //End of drawing
}



void particles()
{
	if (player.pistolEquipped) particleSize = 0.1f;
	else if (player.akEquipped) particleSize = 0.225f;
	else if (player.shotgunEquipped) particleSize = 0.4f;
	else if (player.smgEquipped) particleSize = 0.15f;

	//Blood particle effect
    bloodParams.position = enemyCollision;
    bloodParams.spawnCount = 1;
    bloodParams.minSpeed = 0.5f;
    bloodParams.maxSpeed = 1.0f;
    bloodParams.startColor = MAROON;
    bloodParams.endColor = MAROON;
    bloodParams.minLifetime = 2.0f;
    bloodParams.maxLifetime = 2.0f;
    bloodParams.gravity = { 0.0f, -3.5f, 0.0 };
    bloodParams.startSize = particleSize;
    bloodParams.endSize = particleSize;
    bloodParams.maxAngle = 180.0f;
    bloodParams.fadeAlpha = true;
    bloodParams.enableRandomRotation = true;

    bloodParams.texture = &bloodTexture;
}




void debugControls()
{
    if (IsKeyPressed(KEY_SPACE))
    {
        paused = !paused; // Toggle pause state
    }
    if (IsKeyPressed(KEY_F1))
    {
        debugMode = !debugMode; // Toggle pause state
    }
    if (IsKeyPressed(KEY_F2))
    {
        stopEnemy = !stopEnemy; // Toggle pause state
    }
    if (IsKeyPressed(KEY_F3))
    {
        enemyMove = !enemyMove; // Toggle pause state
    }
    if (IsKeyPressed(KEY_F4))
    {
        stopParticles = !stopParticles; // Toggle pause state
    }
}


void debug()
{
    //>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    ///  Bounding Boxes

    if (debugMode)
    {
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
        //DrawSphere(enemyCollision, 0.1f, BLUE);
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