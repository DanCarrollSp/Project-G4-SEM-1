#include "Main.h"


//Texture assignments
void InitTextures()
{
	//Tile textures
    floorTexture = LoadTexture("resources/World/floorTexture.png");
    ceilingTexture = LoadTexture("resources/World/ceilingTexture.png");
    wallTexture = LoadTexture("resources/World/wallTexture.png");
    doorTexture = LoadTexture("resources/World/door.png");

    //Particle textures
    bloodTexture = LoadTexture("resources/bt80.png");
    shellCasing = LoadTexture("resources/m.png");

	//Model textures
    barrelTexture = LoadTexture("resources/barrel.png");

	//Decal textures
    bulletHole = LoadTexture("resources/bulletHole.png");

	//UI textures
    gameUI.LoadAssets("resources/fonts/Blood.ttf", "resources/fonts/Ammo.otf", "resources/images/ak_icon.png");
}



int main(void)
{
    //Creates the fullscreen window and sets target fps
    InitWindow(screenWidth, screenHeight, "Soul Chain");
    ToggleFullscreen();
    SetTargetFPS(60);
    InitAudioDevice();



    //Sets the mouse position to the center of the screen on start (stops game starting with you looking at the ceiling)
    SetMousePosition(screenWidth / 2, screenHeight / 2);
    //Sets up raylibs perspective camera
    camera.position = Vector3{ 10, 0.6f, 10 };//Camera pos
    camera.up = Vector3{ 0.0f, 1.0f, 0.0f };//Camera orientation
    camera.fovy = 45.0f;//Fov In degrees
    camera.projection = CAMERA_PERSPECTIVE;



    //Load textures
    InitTextures();



    //Seed random time for map generation
    srand((unsigned int)time(NULL));
    //Generate random map
    Image imMap = world.GenerateProceduralMap(MAP_WIDTH, MAP_LENGHT);
    //miniMap = LoadTextureFromImage(imMap);//Minimap
    //Map creation using walls, doors, etc
    mapPixels = LoadImageColors(imMap);//Color map, converts 'image' pixel color data into map data for collisions (black = passavble, else = not passable)
    //Generate nav gird
    navGrid = world.CreateNavigationGrid();

	

    //Shaders
    alphaShader = LoadShader(NULL, "shaders/alpha.fs");
    //Particle setup
    particles();


    
    //Temp
    spawner.Spawn(0, { 5.0f, 0.5f, 5.0f });
    spawner.Spawn(0, { 2.0f, 0.5f, 10.0f });
    spawner.Spawn(0, { 8.0f, 0.5f, 10.0f });
    spawner.Spawn(0, { 5.0f, 0.5f, 5.0f });
    spawner.Spawn(0, { 2.0f, 0.5f, 10.0f });
    spawner.Spawn(0, { 8.0f, 0.5f, 10.0f });



    //Program Loop
    while (!WindowShouldClose())
    {

        //Main menu >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
        if (scenes.sceneMainMenu)
        {
            scenes.Update();
            scenes.Draw(camera);
        }
        // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>



        //Particles >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
        if (scenes.sceneParticleEngine)
        {
            //Particle Engine
            particleEngine.Update(GetFrameTime());
            particleEngine.Draw(camera);
            //Particle system
            particleSystem.UpdateAll(GetFrameTime());
            particleSystem.DrawAll(camera);
        }
        // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>



        //WorldEditor >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
        if (scenes.sceneMapMaker)
        {
			//Inits the world editor once
            if (!mapMakerInit)
            {
				worldEditor.Init();//Inits the editor palettes and assets
                mapMakerInit = true;
            }


            //Update and draw
            worldEditor.HandleInput(camera);
            worldEditor.Draw3D(camera);
			//If testing the level, start the game in level editor level
            if (IsKeyPressed(KEY_T) && IsKeyDown(KEY_LEFT_CONTROL)) testLevel();
        }
        else
        {
			//If the level editor is closed, un-init it
            mapMakerInit = false;
        }
        // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>



        //Gameplay >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
        if (scenes.sceneGameplay)
        {
            //Disables the cursor to lock the mouse to the screen (once)
            if (mouseDisabled == false)
            {
                DisableCursor();
                mouseDisabled = true;
            }

            //Update the game when not paused and always draw the game
            if (!paused) Update();
            Draw();
			//Keep the debugControls outside of the update loop so it can run regardless of if the game is paused or not
            debugControls();
        }
        //>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    }

    //Close program
    if (!WindowShouldClose) CloseWindow();
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
    for (auto& enemy : enemies) if (enemy.IsAlive())player.PreventBoundingBoxCollision(enemy.GetBoundingBox(), player.hitbox, camera, oldCamPos);//Prevent player from walking through bounding boxes


    //Player controls
    player.update(camera);
    player.HandleInput();
    player.closeToWallCheck(camera, world.GetWallBoundingBoxes());
	//Player shooting
	shooting();

    
    //Enemy AI ()
    for (auto& enemy : enemies)
    {
        //Updates enemy AI and movement unless commanded to stop
		if (!stopEnemy)enemy.Update();

		//If the enemy is alive, check for collisions with the player, obstacles and other alive enemies
        if (enemy.IsAlive())
        {
            //Check which enemies this enemy should collide with
            std::vector<Enemy*> otherEnemies;
            for (auto& other : enemies) if (&enemy != &other && other.IsAlive()) otherEnemies.push_back(&other);

			//Move the enemy with the navgrid, colliding with the other enemies (if alive) and the walls
            if (!enemyMove)enemy.Move(player.position, navGrid, world.GetWallBoundingBoxes(), otherEnemies, GetFrameTime());
        }
    }
    //spawner.DespawnDeadEnemies();
    


    //Particles
    particleSystem.UpdateAll(GetFrameTime());
    particles();
    //Decals
    decalManager.Update(GetFrameTime());

}


void Draw()
{
    BeginDrawing();
    ClearBackground(BLACK);//Clears screen
    //>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    BeginMode3D(camera);//Start of 3D Rendering



    //World drawing (walls, doors, floor, ceiling)
    //Draw every layer from file
    const int LEVELS = world.mapHeight / MAP_LENGHT;//mapHeight filled in from World::BuildFromImage
    for (int layer = 0; layer < LEVELS; ++layer)
    {
        //Cube center
        float worldY = layer + 0.5f;

		//Loop through the map and draw the walls, doors, etc
        for (int z = 0; z < MAP_LENGHT; ++z)
        {
            for (int x = 0; x < MAP_WIDTH; ++x)
            {
				//Get the current pixel color from the map
                int index = (layer * MAP_LENGHT + z) * MAP_WIDTH + x;
                Color pixelColor = mapPixels[index];
				//Get the world position of the current pixel
                Vector3 cellPos = { x + 0.5f, worldY, z + 0.5f };



                //Empty space = black
                bool isEmpty = (pixelColor.r == BLACK.r && pixelColor.g == BLACK.g && pixelColor.b == BLACK.b);

                //Walls
                if (pixelColor.r == WHITE.r && pixelColor.g == WHITE.g && pixelColor.b == WHITE.b)
                    globals.DrawCubeTexture(wallTexture, cellPos, 1, 1, 1, WHITE);

                //Doors
                else if (pixelColor.r == BLUE.r && pixelColor.g == BLUE.g && pixelColor.b == BLUE.b)
                    globals.DrawCubeTexture(doorTexture, cellPos, 1, 1, 1, WHITE);

                //Floor
                else if (pixelColor.r == BROWN.r && pixelColor.g == BROWN.g && pixelColor.b == BROWN.b)
                    globals.DrawCubeTexture(floorTexture, cellPos, 1, 1, 1, WHITE);

                //Ceiling
                else if (pixelColor.r == DARKBROWN.r && pixelColor.g == DARKBROWN.g && pixelColor.b == DARKBROWN.b)
                    globals.DrawCubeTexture(ceilingTexture, cellPos, 1, 1, 1, WHITE);


                //Stairs (alternate directions)
                if (ColorEq(pixelColor, ORANGE_N))//N NORTH
                    globals.DrawStair(cellPos, 0, floorTexture);
                else if (ColorEq(pixelColor, ORANGE_S))//S SOUTH
                    globals.DrawStair(cellPos, 1, floorTexture);
                else if (ColorEq(pixelColor, ORANGE_E))//E EAST
                    globals.DrawStair(cellPos, 2, floorTexture);
                else if (ColorEq(pixelColor, ORANGE_W))//W WEST
                    globals.DrawStair(cellPos, 3, floorTexture);

            }
        }
    }



    /// Draw entites /////>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    BeginShaderMode(alphaShader);
    debug();//Debugging visuals ( example - shows box collider outlines and raycasts)

	//Crosshair
    DrawSphere(crosshair, 0.0002f, crosshairColor);
	//Draws the Decals
    decalManager.Draw();

    for (auto& enemy : enemies) if (!stopEnemy)enemy.Draw(camera);//Draws the enemy, camera for billboarding
    particleSystem.DrawAll(camera);//Draws all active particle effects


    EndShaderMode();
    EndMode3D();
    //Start of drawing//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


    //Draws the minimap
    world.DrawMiniMapSmall(player, world, screenWidth, screenHeight);

    //Draws the players hand and updates its animations
    player.Animate(screenWidth, screenHeight, camera, mapPosition);

    //UI
    //auto [currentAmmo, maxAmmo] = player.GetAmmo();
    auto ammo = player.GetAmmo();
    auto currentAmmo = std::get<0>(ammo);
    auto maxAmmo = std::get<1>(ammo);
    gameUI.Draw(100, currentAmmo, maxAmmo);
    gameUI.DrawStatic();

    if (paused) DrawTexture(gameUI.pauseTexture, 0, 0, WHITE);//Pause menu

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
    bloodParams.gravity = { 0.0f, -6.5f, 0.0 };
    bloodParams.startSize = particleSize;
    bloodParams.endSize = particleSize;
    bloodParams.maxAngle = 180.0f;
    bloodParams.fadeAlpha = true;
    bloodParams.enableRandomRotation = true;

    bloodParams.texture = &bloodTexture;


    //Muzzle flash particle effect
    Vector3 forward = Vector3Normalize(Vector3Subtract(camera.target, camera.position)); // Direction camera is facing
    Vector3 right = Vector3Normalize(Vector3CrossProduct(forward, camera.up)); // Right vector

    // Offset from camera position: forward + right (and maybe a little downward too)
    Vector3 spawnOffset = Vector3Add(
        Vector3Scale(forward, 0.3f),    // 0.3 units in front
        Vector3Scale(right, 0.09f)      // 0.15 units to the right
    );
    spawnOffset.y -= 0.03f;             // slight downward offset

    shellCasingParams.position = Vector3Add(camera.position, spawnOffset);

    shellCasingParams.spawnCount = 1;
    shellCasingParams.minSpeed = 0.5f;
    shellCasingParams.maxSpeed = 1.0f;
    shellCasingParams.startColor = MAROON;
    shellCasingParams.endColor = MAROON;
    shellCasingParams.minLifetime = 2.0f;
    shellCasingParams.maxLifetime = 2.0f;
    shellCasingParams.gravity = { 0.0f, -3.0f, 0.0 };
    shellCasingParams.startSize = particleSize / 4;
    shellCasingParams.endSize = particleSize / 4;
    shellCasingParams.maxAngle = 180.0f;
    shellCasingParams.fadeAlpha = true;
    shellCasingParams.enableRandomRotation = true;

    shellCasingParams.texture = &shellCasing;
}



void shooting()
{
	//Handles actual shooting
    if (player.justFired)
    {
        ProcessBulletShot(camera, world.GetWallBoundingBoxes(), world.GetDoorBoundingBoxes(),
                            enemies, player.pistolEquipped, player.akEquipped, player.shotgunEquipped, player.smgEquipped);
    }


	//Crosshair
    // 
    //Draws a circle crosshair on the first piece of a raycasted line to represent the crosshair
    Vector3 rayDirection = Vector3Normalize(Vector3Subtract(camera.target, camera.position));
    crosshair = Vector3Add(camera.position, Vector3Scale(rayDirection, 0.1f));

    //Makes the crosshair red while aiming at enemies, also makes sure we know which enemy were looking at for collisions
    Vector3 firstEnemyCollision = { 0, 0, 0 };
    for (auto& enemy : enemies)
    {
        Vector3 col = player.calcBulletCollision(camera, enemy.hitbox);
        //If a collision is detected, store it and break out of the loop.
        if (chechVec3(col, { 0, 0, 0 }))
        {
            firstEnemyCollision = col;
            break;
        }
    }
    enemyCollision = firstEnemyCollision;

    //If the player looks at an enemy, change the crosshair color to red unless there are obsticles in the way
    if (IsEnemyInSight(camera, world.GetWallBoundingBoxes(), world.GetDoorBoundingBoxes(), enemies)) crosshairColor = RED;
    else crosshairColor = WHITE;
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


    //>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    /// Raycast hit points
        Vector3 zero = { 0, 0, 0 };

        //Wall-bullet collision point
        for (int i = 0; i < world.GetWallBoundingBoxes().size(); i++)
        {
            Vector3 wallCollision = player.calcBulletCollision(camera, world.GetWallBoundingBoxes()[i]);
            if (chechVec3(enemyCollision, zero)) DrawSphere(wallCollision, 0.1f, BLUE);
        }
        //Door-bullet collision point
        for (int i = 0; i < world.GetDoorBoundingBoxes().size(); i++)
        {
            Vector3 doorCollision = player.calcBulletCollision(camera, world.GetDoorBoundingBoxes()[i]);
            if (chechVec3(enemyCollision, zero)) DrawSphere(doorCollision, 0.1f, GREEN);
        }


        //Draws a RED sphere on a shot walls (drawn over by blue hit detection for now)
        if (player.hitTarget && player.shot)DrawSphere(player.hitPoint, 0.1f, RED);

        DrawSphere(enemyCollision, 0.1f, BLUE);
    //>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    }
}



void testLevel()
{
    //Get the player spawn point from spawn blocks in the level
    Vector3 spawn;
    if (!worldEditor.FindPlayerSpawn(spawn))
    {
        TraceLog(LOG_WARNING, "No BLK_SPAWN tile – cannot start level");
    }
    else//Spawn point found
    {
        ///Build the level from the save file
        //First unload any map data
        if (miniMap.id) UnloadTexture(miniMap);
        if (mapPixels) UnloadImageColors(mapPixels);
        //Then load in the map data from file
        Image tall = worldEditor.BakeImage();
        miniMap = LoadTextureFromImage(tall);
        mapPixels = LoadImageColors(tall);

        //Build the world from the image
        world.BuildFromImage(tall);//Place all the tiles from the pixel data
        navGrid = world.CreateNavigationGrid();//Create the navigation grid for pathfinding
        UnloadImage(tall);//Save file no longer needed

        //Spawn the player on the found spawn point and set its initial physics
        camera.position = spawn;
        player.velocityY = 0.0f;
        player.doGravity = false;//Start as false so world can be built correctly before player starts gravity calculations


        //Close the level editor and begin gameplay
        scenes.sceneMapMaker = false;
        scenes.sceneGameplay = true;
        mapMakerInit = false;//So level editor will reinitialize next time
        EnableCursor();//No cursor 
    }
}





bool chechVec3(const Vector3& vec1, const Vector3& vec2)
{
    return (vec1.x != vec2.x || vec1.y != vec2.y || vec1.z != vec2.z);
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