#include "Main.h"

//Texture assignments
void InitTextures()
{
	//Tile textures
    initWorldTextures();

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
    //UpdateCamera(&camera, CAMERA_FIRST_PERSON);//Update the camera (movement)
    float dt = GetFrameTime();
    player.UpdateLookAngles();
    player.MoveAndCollide(dt, camera, world.GetWallBoundingBoxes(), world.GetDoorBoundingBoxes());
    player.ApplyLook(camera);
    player.UpdateFOV(camera, dt);

    //Collision
    //player.PreventBoundingBoxCollisions(world.GetWallBoundingBoxes(), player.hitbox, camera, oldCamPos);//Prevent player from walking through bounding boxes
    for (auto& enemy : enemies) if (enemy.IsAlive())player.PreventBoundingBoxCollision(enemy.GetBoundingBox(), player.hitbox, camera, oldCamPos);//Prevent player from walking through bounding boxes


    //Player controls
    player.update(camera);
    player.HandleInput();
    //player.closeToWallCheck(camera, world.GetWallBoundingBoxes());
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

    Doors::Update();
}


void Draw()
{
    BeginDrawing();
    ClearBackground(BLACK);//Clears screen
    //>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    BeginMode3D(camera);//Start of 3D Rendering

    Doors::Draw();

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
                Color color = mapPixels[index];
				//Get the world position of the current pixel
                Vector3 pos = { x + 0.5f, worldY, z + 0.5f };



                //Empty space = black
                bool isEmpty = (color.r == BLACK.r && color.g == BLACK.g && color.b == BLACK.b);

                //Determine texture to draw based on block color
                if (ColorEq(color, WallColor)) globals.DrawCubeTexture(TextureWall, pos, 1, 1, 1, WHITE);
                else if (ColorEq(color, DoorColor1)) globals.DrawCubeTexture(TextureDoor, pos, 0.2, 1, 1, WHITE);
                else if (ColorEq(color, DoorColor2)) globals.DrawCubeTexture(TextureDoor, pos, 1, 1, 0.2, WHITE);
                else if (ColorEq(color, FloorColor)) globals.DrawCubeTexture(TextureFloor, pos, 1, 1, 1, WHITE);
                else if (ColorEq(color, StoneColor)) globals.DrawCubeTexture(TextureStone, pos, 1, 1, 1, WHITE);
                else if (ColorEq(color, TEST_COLOR)) globals.DrawCubeTexture(TEST_TEXTURE, pos, 1, 1, 1, WHITE);

                //else if (ColorEq(color, SpawnColor)) globals.DrawCubeTexture(TextureSpawn, pos, 1, 1, 1, WHITE);

                //Brick
                else if (ColorEq(color, Brick1Color)) globals.DrawCubeTexture(TextureBrick1, pos, 1, 1, 1, WHITE);
                else if (ColorEq(color, Brick2Color)) globals.DrawCubeTexture(TextureBrick2, pos, 1, 1, 1, WHITE);
                else if (ColorEq(color, Brick3Color)) globals.DrawCubeTexture(TextureBrick3, pos, 1, 1, 1, WHITE);
                else if (ColorEq(color, Brick4Color)) globals.DrawCubeTexture(TextureBrick4, pos, 1, 1, 1, WHITE);

                //Brick Faces
                else if (ColorEq(color, BrickFace1Color)) globals.DrawCubeTexture(TextureBrickFace1, pos, 1, 1, 1, WHITE);
                else if (ColorEq(color, BrickFace2Color)) globals.DrawCubeTexture(TextureBrickFace2, pos, 1, 1, 1, WHITE);
                else if (ColorEq(color, BrickFace2BloodColor)) globals.DrawCubeTexture(TextureBrickFace2Blood, pos, 1, 1, 1, WHITE);
                else if (ColorEq(color, BrickFace3Color)) globals.DrawCubeTexture(TextureBrickFace3, pos, 1, 1, 1, WHITE);
                else if (ColorEq(color, BrickFace3BloodColor)) globals.DrawCubeTexture(TextureBrickFace3Blood, pos, 1, 1, 1, WHITE);
                else if (ColorEq(color, BrickFace4Color)) globals.DrawCubeTexture(TextureBrickFace4, pos, 1, 1, 1, WHITE);
                else if (ColorEq(color, BrickFace5Color)) globals.DrawCubeTexture(TextureBrickFace5, pos, 1, 1, 1, WHITE);
                else if (ColorEq(color, RedBrickTextureColor)) globals.DrawCubeTexture(TextureRedBrickTexture, pos, 1, 1, 1, WHITE);

                //Cement
                else if (ColorEq(color, CementWallColor)) globals.DrawCubeTexture(TextureCementWall, pos, 1, 1, 1, WHITE);
                else if (ColorEq(color, CementWallBottom1Color)) globals.DrawCubeTexture(TextureCementWallBottom1, pos, 1, 1, 1, WHITE);
                else if (ColorEq(color, CementWallBottom2Color)) globals.DrawCubeTexture(TextureCementWallBottom2, pos, 1, 1, 1, WHITE);
                else if (ColorEq(color, CementWallBottom3Color)) globals.DrawCubeTexture(TextureCementWallBottom3, pos, 1, 1, 1, WHITE);
                else if (ColorEq(color, CementWallDeco1Color)) globals.DrawCubeTexture(TextureCementWallDeco1, pos, 1, 1, 1, WHITE);
                else if (ColorEq(color, CementWallDeco2Color)) globals.DrawCubeTexture(TextureCementWallDeco2, pos, 1, 1, 1, WHITE);
                else if (ColorEq(color, CementWallTop1Color)) globals.DrawCubeTexture(TextureCementWallTop1, pos, 1, 1, 1, WHITE);
                else if (ColorEq(color, CementWallTop2Color)) globals.DrawCubeTexture(TextureCementWallTop2, pos, 1, 1, 1, WHITE);

                //Dark Steel
                else if (ColorEq(color, DarkSteelColor)) globals.DrawCubeTexture(TextureDarkSteel, pos, 1, 1, 1, WHITE);
                else if (ColorEq(color, DarkSteelBeamColor)) globals.DrawCubeTexture(TextureDarkSteelBeam, pos, 1, 1, 1, WHITE);
                else if (ColorEq(color, DarkSteelDecoColor)) globals.DrawCubeTexture(TextureDarkSteelDeco, pos, 1, 1, 1, WHITE);
                else if (ColorEq(color, DarkSteelHazardColor)) globals.DrawCubeTexture(TextureDarkSteelHazard, pos, 1, 1, 1, WHITE);
                else if (ColorEq(color, DarkSteelTop1Color)) globals.DrawCubeTexture(TextureDarkSteelTop1, pos, 1, 1, 1, WHITE);
                else if (ColorEq(color, DarkSteelTop2Color)) globals.DrawCubeTexture(TextureDarkSteelTop2, pos, 1, 1, 1, WHITE);
                else if (ColorEq(color, DarkSteelWallColor)) globals.DrawCubeTexture(TextureDarkSteelWall, pos, 1, 1, 1, WHITE);
                else if (ColorEq(color, DarkSteelWallDecoColor)) globals.DrawCubeTexture(TextureDarkSteelWallDeco, pos, 1, 1, 1, WHITE);

                //Steel
                else if (ColorEq(color, OldSteelColor)) globals.DrawCubeTexture(TextureOldSteel, pos, 1, 1, 1, WHITE);
                else if (ColorEq(color, RustBeamUpColor)) globals.DrawCubeTexture(TextureRustBeamUp, pos, 1, 1, 1, WHITE);
                else if (ColorEq(color, RustBeamSideColor)) globals.DrawCubeTexture(TextureRustBeamSide, pos, 1, 1, 1, WHITE);
                else if (ColorEq(color, SteelBeam1Color)) globals.DrawCubeTexture(TextureSteelBeam1, pos, 0.4, 1, 0.4, WHITE);
                else if (ColorEq(color, SteelBeam2Color)) globals.DrawCubeTexture(TextureSteelBeam2, pos, 0.4, 1, 0.4, WHITE);
                else if (ColorEq(color, SteelDoorColor)) globals.DrawCubeTexture(TextureSteelDoor, pos, 1, 1, 1, WHITE);
                else if (ColorEq(color, SteelGrip1Color)) globals.DrawCubeTexture(TextureSteelGrip1, pos, 1, 1, 1, WHITE);
                else if (ColorEq(color, SteelGrip2Color)) globals.DrawCubeTexture(TextureSteelGrip2, pos, 1, 1, 1, WHITE);
                else if (ColorEq(color, SteelPlateColor)) globals.DrawCubeTexture(TextureSteelPlate, pos, 1, 1, 1, WHITE);
                else if (ColorEq(color, SteelReinforcedColor)) globals.DrawCubeTexture(TextureSteelReinforced, pos, 1, 1, 1, WHITE);

                //Grates
                else if (ColorEq(color, GrateColor)) globals.DrawCubeTexture(TextureGrate, pos, 1, 1, 1, WHITE);
                else if (ColorEq(color, GrateRustColor)) globals.DrawCubeTexture(TextureGrateRust, pos, 1, 1, 1, WHITE);

                



                //Stairs (alternate directions)
                if (ColorEq(color, ORANGE_N))//N NORTH
                    globals.DrawStair(pos, 0, TEST_TEXTURE);
                else if (ColorEq(color, ORANGE_S))//S SOUTH
                    globals.DrawStair(pos, 1, TEST_TEXTURE);
                else if (ColorEq(color, ORANGE_E))//E EAST
                    globals.DrawStair(pos, 2, TEST_TEXTURE);
                else if (ColorEq(color, ORANGE_W))//W WEST
                    globals.DrawStair(pos, 3, TEST_TEXTURE);

            }
        }
    }



    /// Draw entites /////>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    BeginShaderMode(alphaShader);
    debug();//Debugging visuals ( example - shows box collider outlines and raycasts)

    //Draw world voxels that may be opaque
    for (int layer = 0; layer < LEVELS; ++layer)
        for (int z = 0; z < MAP_LENGHT; ++z)
            for (int x = 0; x < MAP_WIDTH; ++x)
            {
                int index = (layer * MAP_LENGHT + z) * MAP_WIDTH + x;
                Color color = mapPixels[index];
                if (color.a == 0 || ColorEq(color, BLACK)) continue;

                Vector3 pos = { x + 0.5f, layer + 0.5f, z + 0.5f };
                Vector3 bigDoorPos = pos;
                bigDoorPos.y += 0.5;

                // only the misc “alpha” textures here:
                if (ColorEq(color, SupportBeamColor)) globals.DrawCubeTexture(TextureSupportBeam, pos, .33f, 1, .33f, WHITE);
                else if (ColorEq(color, SwitchGreenColor)) globals.DrawCubeTexture(TextureSwitchGreen, pos, 1, 1, 1, WHITE);
                else if (ColorEq(color, SwitchRedColor)) globals.DrawCubeTexture(TextureSwitchRed, pos, 1, 1, 1, WHITE);
                else if (ColorEq(color, BigDoorLeftColor1)) globals.DrawCubeTexture(TextureBigDoorLeft, bigDoorPos, 1, 2, 0.2, WHITE);//
                else if (ColorEq(color, BigDoorRightColor1))globals.DrawCubeTexture(TextureBigDoorRight, bigDoorPos, 1, 2, 0.2, WHITE);
                else if (ColorEq(color, BigDoorLeftColor2)) globals.DrawCubeTexture(TextureBigDoorLeft, bigDoorPos, 0.2, 2, 1, WHITE);//
                else if (ColorEq(color, BigDoorRightColor2))globals.DrawCubeTexture(TextureBigDoorRight, bigDoorPos, 0.2, 2, 1, WHITE);
                else if (ColorEq(color, FenceColor)) globals.DrawCubeTexture(TextureFence, pos, .01f, 1, 1, WHITE);
                else if (ColorEq(color, VentColor)) globals.DrawCubeTexture(TextureVent, pos, 1, 1, 1, WHITE);
            }

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