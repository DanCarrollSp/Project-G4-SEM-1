#include "WorldEditor.h"




/// /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Constructor and Basic setups/initing
//

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//Constructor to scan for existing maps, load or create blanks, init UI
WorldEditor::WorldEditor()
{
    //Container for pairs of map indexs and file paths
    std::vector<std::pair<int, std::string>> found;
    //Iterate through the maps folder to find PNG map files
    for (auto& file : std::filesystem::directory_iterator("resources/maps"))
    {
        //Skip any file that doesnt have a .png extension
        if (file.path().extension() == ".png")
        {
            int index;

            //Attempt getting filenames with the name formatting custom_??.png
            if (sscanf_s(file.path().filename().string().c_str(), "custom_%02d.png", &index) == 1)
            {
                found.emplace_back(index, file.path().string());//Store found map and its index
            }
        }
    }

    //Sort the found maps by their index so they are in the correct order
    std::sort(found.begin(), found.end(), [](auto& a, auto& b) { return a.first < b.first; });

    //If no custom map found, use a default map at index 0
    //else, add each found maps path to the list of map paths
    if (found.empty()) mapPaths.push_back(DefaultMapPath(0));
    else for (auto& p : found) mapPaths.push_back(p.second);

    //Resize the maps vector to match the number of maps found
    maps.resize(mapPaths.size());

    //Attempt to load each map, if loading fails, create a blank one
    for (int i = 0; i < (int)maps.size(); ++i)
    {
        if (!LoadMap(i))
        {
            //Fail to load, create a blank map in its place (wont be saved and will attempt to be reloaded when switched to)
            //Sets the dimensions and set all voxel data to empty (black)
            maps[i].width = MAP_WIDTH;
            maps[i].height = MAP_LENGHT;
            maps[i].levels = MAP_LEVELS;
            maps[i].voxels.assign(size_t(MAP_WIDTH) * MAP_LENGHT * MAP_LEVELS, BLACK);
        }
    }

    //Initialize UI commands after maps are ready
    InitCommands();
    //Set starting map index, level and reset dragging state
    currentMapIndex = 0;
    currentLevel = 0;
    dragLayer = -1;
}

//Init Palette separate from constructor as it needs to load textures
void WorldEditor::Init()
{
    InitPalette();

    alphaShader = LoadShader(NULL, "shaders/alpha.fs");
}

//Setup default file path for maps
std::string WorldEditor::DefaultMapPath(int i)
{
    //Format the path string for the map file (raylib wont take in a string for LoadImage())
    char buf[64];
    sprintf_s(buf, "resources/maps/custom_%02d.png", i);
    return buf;
}




/// /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Helper functions
//


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//Assigns a block a color for saving to the image
Color WorldEditor::BlockToPixel(Block b)
{
    //Map each block type to its corrasponding pixel color
    switch (b)
    {
        // Default tiles
    case BLK_WALL:             return WallColor;
    case BLK_DOOR1:             return DoorColor1;
    case BLK_DOOR2:             return DoorColor2;
    case BLK_FLOOR:            return FloorColor;
    case BLK_STONE:            return StoneColor;
    case BLK_TEST:             return TEST_COLOR;

        // Utility tiles
    case BLK_SPAWN:            return SpawnColor;

        // Stairs
    case BLK_STAIR_N:          return ORANGE_N;
    case BLK_STAIR_S:          return ORANGE_S;
    case BLK_STAIR_E:          return ORANGE_E;
    case BLK_STAIR_W:          return ORANGE_W;

        // Brick
    case BLK_BRICK1:           return Brick1Color;
    case BLK_BRICK2:           return Brick2Color;
    case BLK_BRICK3:           return Brick3Color;
    case BLK_BRICK4:           return Brick4Color;

        // Brick Faces
    case BLK_BRICK_FACE1:      return BrickFace1Color;
    case BLK_BRICK_FACE2:      return BrickFace2Color;
    case BLK_BRICK_FACE2_BLOOD:return BrickFace2BloodColor;
    case BLK_BRICK_FACE3:      return BrickFace3Color;
    case BLK_BRICK_FACE3_BLOOD:return BrickFace3BloodColor;
    case BLK_BRICK_FACE4:      return BrickFace4Color;
    case BLK_BRICK_FACE5:      return BrickFace5Color;
    case BLK_RED_BRICK:        return RedBrickTextureColor;

        // Cement / Concrete
    case BLK_CEMENT_WALL:      return CementWallColor;
    case BLK_CEMENT_BOTTOM1:   return CementWallBottom1Color;
    case BLK_CEMENT_BOTTOM2:   return CementWallBottom2Color;
    case BLK_CEMENT_BOTTOM3:   return CementWallBottom3Color;
    case BLK_CEMENT_DECO1:     return CementWallDeco1Color;
    case BLK_CEMENT_DECO2:     return CementWallDeco2Color;
    case BLK_CEMENT_TOP1:      return CementWallTop1Color;
    case BLK_CEMENT_TOP2:      return CementWallTop2Color;

        // Dark Steel
    case BLK_DARK_STEEL:           return DarkSteelColor;
    case BLK_DARK_STEEL_BEAM:      return DarkSteelBeamColor;
    case BLK_DARK_STEEL_DECO:      return DarkSteelDecoColor;
    case BLK_DARK_STEEL_HAZARD:    return DarkSteelHazardColor;
    case BLK_DARK_STEEL_TOP1:      return DarkSteelTop1Color;
    case BLK_DARK_STEEL_TOP2:      return DarkSteelTop2Color;
    case BLK_DARK_STEEL_WALL:      return DarkSteelWallColor;
    case BLK_DARK_STEEL_WALL_DECO:return DarkSteelWallDecoColor;

        // Steel
    case BLK_OLD_STEEL:        return OldSteelColor;
    case BLK_RUST_BEAM_UP:     return RustBeamUpColor;
    case BLK_RUST_BEAM_SIDE:   return RustBeamSideColor;
    case BLK_STEEL_BEAM1:      return SteelBeam1Color;
    case BLK_STEEL_BEAM2:      return SteelBeam2Color;
    case BLK_STEEL_DOOR:       return SteelDoorColor;
    case BLK_STEEL_GRIP1:      return SteelGrip1Color;
    case BLK_STEEL_GRIP2:      return SteelGrip2Color;
    case BLK_STEEL_PLATE:      return SteelPlateColor;
    case BLK_STEEL_REINFORCED: return SteelReinforcedColor;

        // Grates
    case BLK_GRATE:            return GrateColor;
    case BLK_GRATE_RUST:       return GrateRustColor;

        // Misc
    case BLK_SUPPORT_BEAM:      return SupportBeamColor;
    case BLK_SWITCH_GREEN:      return SwitchGreenColor;//
    case BLK_SWITCH_RED:        return SwitchRedColor;
    case BLK_BIG_DOOR_LEFT1:     return BigDoorLeftColor1;//
    case BLK_BIG_DOOR_RIGHT1:    return BigDoorRightColor1;
    case BLK_BIG_DOOR_LEFT2:     return BigDoorLeftColor2;//
    case BLK_BIG_DOOR_RIGHT2:    return BigDoorRightColor2;
    case BLK_FENCE:             return FenceColor;
    case BLK_VENT:              return VentColor;


        // Empty / Unknown
    default:                   return BLACK;

    }
}

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//Find and loop through all the player spawn points in the level on load, randomly select one to spawn the player at
bool WorldEditor::FindPlayerSpawn(Vector3& out)
{
    //Current map reference
    const Map& m = maps[currentMapIndex];
    //Vector to store all spawn points
    std::vector<std::tuple<int, int, int>> spawns;

    //Iterate through every voxel to find spawn points
    for (int l = 0; l < m.levels; ++l)//layers
        for (int z = 0; z < m.height; ++z)//rows
            for (int x = 0; x < m.width; ++x)//columns
            {
                //Get the color of the pixel of the voxel at the current coordinates
                Color v = m.voxels[coord3dToIndex(l, z, x)];
                //Checks if its a spawn pixel by color (r=2, g=1, b=1)
                if (ColorEq(v, SpawnColor)) spawns.emplace_back(l, z, x);//Record spawn point in the vector
            }

    //Return false if no spawn points were found
    if (spawns.empty()) return false;


    //If we get this far, we have at least one spawn point
    //Choose a random spawn point from the list
    std::mt19937 random{ std::random_device{}() };
    std::uniform_int_distribution<int> d(0, int(spawns.size()) - 1);
    auto [ly, z, x] = spawns[d(random)];
    //Converts the grid position to world coordinates with player size offset on Y
    out = { x + .5f, ly + .51f, z + .5f };
    return true;
}

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//Creates a 2D image map of the 3D world by flattening its levels of width x height and stacking each slice vertically
Image WorldEditor::BakeImage()
{
    //Grab the current map
    Map& currentMap = maps[currentMapIndex];
    //Create an image tall enough to stack all levels vertically
    Image img = GenImageColor(currentMap.width, currentMap.height * currentMap.levels, BLACK);
    Color* destination = (Color*)img.data;

    //Copy voxel colors into the image layer by layer
    for (int l = 0; l < currentMap.levels; ++l)
        for (int z = 0; z < currentMap.height; ++z)
            for (int x = 0; x < currentMap.width; ++x)
            {
                destination[(l * currentMap.height + z) * currentMap.width + x] = currentMap.voxels[coord3dToIndex(l, z, x)];
            }

    //Return the generated image
    return img;
}




/// /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Drawing the 3D environment and Handling user input
//

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//3D rendering of voxels and additional UI (current map and layer selected)
void WorldEditor::Draw3D(Camera3D& cam)
{
    //Setup rendering
    BeginDrawing();
    ClearBackground(WHITE);
    BeginMode3D(cam);

    //Get the current map
    const Map& currentMap = maps[currentMapIndex];
    //Iterates through all voxels to draw whats in the map from the img
    for (int l = 0; l < currentMap.levels; ++l)
        for (int z = 0; z < currentMap.height; ++z)
            for (int x = 0; x < currentMap.width; ++x)
            {
                //Get the pixel color of the voxel at the current coordinates
                Color color = currentMap.voxels[coord3dToIndex(l, z, x)];

                //Skip fully transparent and empty voxels
                if (color.a == 0) continue;
                if (ColorEq(color, BLACK)) continue;


                //Set the placement position of vocels
                Vector3 pos{ x + .5f, l + .5f, z + .5f };


                //Determine texture to draw based on block color
                if (ColorEq(color, WallColor)) globals.DrawCubeTexture(TextureWall, pos, 1, 1, 1, WHITE);
                else if (ColorEq(color, DoorColor1)) globals.DrawCubeTexture(TextureDoor, pos, 0.2, 1, 1, WHITE);
                else if (ColorEq(color, DoorColor2)) globals.DrawCubeTexture(TextureDoor, pos, 1, 1, 0.2, WHITE);
                else if (ColorEq(color, FloorColor)) globals.DrawCubeTexture(TextureFloor, pos, 1, 1, 1, WHITE);
                else if (ColorEq(color, StoneColor)) globals.DrawCubeTexture(TextureStone, pos, 1, 1, 1, WHITE);
                else if (ColorEq(color, TEST_COLOR)) globals.DrawCubeTexture(TEST_TEXTURE, pos, 1, 1, 1, WHITE);

                else if (ColorEq(color, SpawnColor)) globals.DrawCubeTexture(TextureSpawn, pos, 1, 1, 1, WHITE);

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


                Vector3 bigDoorPos = pos;
                bigDoorPos.y += 0.5;
                //Misc
                BeginShaderMode(alphaShader);
                if (ColorEq(color, SupportBeamColor)) globals.DrawCubeTexture(TextureSupportBeam, pos, .33, 1, .33, WHITE);
                else if (ColorEq(color, SwitchGreenColor)) globals.DrawCubeTexture(TextureSwitchGreen, pos, 1, 1, 1, WHITE);//
                else if (ColorEq(color, SwitchRedColor)) globals.DrawCubeTexture(TextureSwitchRed, pos, 1, 1, 1, WHITE);
                else if (ColorEq(color, BigDoorLeftColor1)) globals.DrawCubeTexture(TextureBigDoorLeft, bigDoorPos, 1, 2, 0.2, WHITE);//
                else if (ColorEq(color, BigDoorRightColor1)) globals.DrawCubeTexture(TextureBigDoorRight, bigDoorPos, 1, 2, 0.2, WHITE);
                else if (ColorEq(color, BigDoorLeftColor2)) globals.DrawCubeTexture(TextureBigDoorLeft, bigDoorPos, 0.2, 2, 1, WHITE);//
                else if (ColorEq(color, BigDoorRightColor2)) globals.DrawCubeTexture(TextureBigDoorRight, bigDoorPos, 0.2, 2, 1, WHITE);
                else if (ColorEq(color, FenceColor)) globals.DrawCubeTexture(TextureFence, pos, .1, 1, 1, WHITE);
                else if (ColorEq(color, VentColor)) globals.DrawCubeTexture(TextureVent, pos, 1, 1, 1, WHITE);
                EndShaderMode();

                //Stairs
                if (ColorEq(color, ORANGE_N) || ColorEq(color, ORANGE_S) || ColorEq(color, ORANGE_E) || ColorEq(color, ORANGE_W))
                {
                    //For stair step segments with varying heights
                    float heights[4]{ .25f, .5f, .75f, 1.0f };
                    //Direction determinats the orientation of the stairs, get the direction from the color
                    int direction = (color.g == 128 ? 0 : color.g == 160 ? 1 : color.g == 192 ? 2 : 3);

                    //Set the placement positioning of each step withing the block based on direction of stair
                    for (int i = 0; i < 4; ++i)
                    {
                        //Set the position of each stair step
                        Vector3 stairPos = pos;
                        //Set the size of each stair segment
                        Vector3 s{ (direction < 2 ? 1 : .25f), heights[i], (direction < 2 ? .25f : 1) };

                        //Offset stair step positioning
                        if (direction == 0) stairPos.z += (i - 1.5f) * .25f;
                        if (direction == 1) stairPos.z += (1.5f - i) * .25f;
                        if (direction == 2) stairPos.x += (1.5f - i) * .25f;
                        if (direction == 3) stairPos.x += (i - 1.5f) * .25f;
                        stairPos.y = floorf(stairPos.y) + heights[i] * .5f;

                        //Draw the stairs
                        globals.DrawCubeTexture(TEST_TEXTURE, stairPos, s.x, s.y, s.z, WHITE);
                    }
                }
            }


    //Draws the grid for reference
    rlPushMatrix();
    rlTranslatef(currentMap.width * .5f, .001f, currentMap.height * .5f);
    DrawGrid(100, 1.0f);
    rlPopMatrix();

    //Ends 3D rendering
    EndMode3D();

    // Draw UI text showing current map and layer
    DrawText(TextFormat("Map %d/%d Layer %d/%d", currentMapIndex + 1, maps.size(), currentLevel, currentMap.levels - 1), 10, 10, 20, BLACK);

    //Draw the palettes
    DrawPalette();
    DrawCommandPalette();
    EndDrawing();//Ends drawing

}

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//Full input handling - UI first (swallowing other actions), then world edits
void WorldEditor::HandleInput(Camera3D& cam)
{
    //Get mouse pos and wheel delta
    Vector2 mousePos = GetMousePosition();
    float wheel = GetMouseWheelMove();


    //Tile palette toggling - open/close
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(mousePos, paletteToggle))
    {
        paletteExpanded = !paletteExpanded;//Toggle state
        paletteScroll = 0.0f;//Reset scroll if toggled
        InitPalette();//Reinit if toggled
        lastX = lastY = lastZ = -1;//Reset drag tracking
        dragLayer = -1;//Cancel any dragging
        return;//Swallow input
    }

    //Tile palette UI - handles scrolling and tile selections
    if (paletteExpanded)
    {
        //Scrolls the palette
        UpdatePaletteScroll(mousePos, wheel);

        //Vars
        float paletteTileWidth = paletteToggle.width - 20.0f;
        float pad = 12.0f;
        float xPos = paletteToggle.x + 10.0f;
        float yPos = paletteToggle.y + paletteToggle.height + pad;
        float viewportHeight = GetScreenHeight() - yPos - pad;


        //Handle tile pick on click
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            for (int i = 0; i < (int)palette.size(); ++i)
            {
                //Palette tiles
                Rectangle rect{ xPos, yPos + i * (paletteTileWidth + pad) - paletteScroll, paletteTileWidth, paletteTileWidth };

                //If clicked on a tile
                if (CheckCollisionPointRec(mousePos, rect))
                {
                    current = palette[i].type;//Set selected tile to the clicked one
                    lastX = lastY = lastZ = -1;//Reset drag
                    dragLayer = -1;
                    return;//Swallow input
                }
            }
        }

        //Prevent clicks behind the palette
        Rectangle uiArea{ xPos, yPos, paletteTileWidth + paletteScrollBar.width + 4, viewportHeight };
        if (CheckCollisionPointRec(mousePos, uiArea)) return;
    }

    //Command palette toggle & UI (open/close commands list)
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(mousePos, cmdsPaletteToggle))
    {
        //Toggle command palette, reset drag layer and swallow input
        cmdsPaletteExpanded = !cmdsPaletteExpanded;
        dragLayer = -1;
        return;
    }
    if (cmdsPaletteExpanded)
    {
        //Handle command clicks
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCommandClicks())
        {
            dragLayer = -1;
            return;
        }

        //Prevent clicks behind the commands UI
        Rectangle cmdArea = cmdsPaletteToggle;
        cmdArea.y += cmdArea.height + 4.0f;
        cmdArea.height = commandButtons.size() * (cmdArea.height + 6.0f);
        if (CheckCollisionPointRec(mousePos, cmdArea)) return;
    }



    //Free look editor camera control with boost
    if (IsMouseButtonDown(MOUSE_BUTTON_SIDE))
    {
        //Do normal look movement
        UpdateCamera(&cam, CAMERA_FREE);

        //If Shift is held, add extra movement
        if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT))
        {
            const int boostAmount = 8;
            const float defaultSpeed = 4.0f;//Raylibs CAMERA_FREE move speed
            float dt = GetFrameTime();

            //Get the forward and right vectors
            Vector3 forward = Vector3Normalize(Vector3Subtract(cam.target, cam.position));
            forward.y = 0;
            forward = Vector3Normalize(forward);
            Vector3 right = Vector3Normalize(Vector3CrossProduct(forward, cam.up));

            //Movement controls
            Vector3 dir = { 0,0,0 };
            if (IsKeyDown(KEY_W)) dir = Vector3Add(dir, forward);
            if (IsKeyDown(KEY_S)) dir = Vector3Subtract(dir, forward);
            if (IsKeyDown(KEY_D)) dir = Vector3Add(dir, right);
            if (IsKeyDown(KEY_A)) dir = Vector3Subtract(dir, right);

            if (Vector3Length(dir) > 0.0f)
            {
                dir = Vector3Normalize(dir);
                //Move with the extra boostAmount
                float extra = defaultSpeed * dt * (boostAmount - 1);
                Vector3 delta = Vector3Scale(dir, extra);
                //Update pos and target
                cam.position = Vector3Add(cam.position, delta);
                cam.target = Vector3Add(cam.target, delta);
            }
        }
    }


    //Layer up/down shortcuts
    if (IsKeyPressed(KEY_E) && currentLevel < maps[currentMapIndex].levels - 1) ++currentLevel;
    if (IsKeyPressed(KEY_Q) && currentLevel > 0) --currentLevel;



    //Capture mouse and key states for editing
    bool shift = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);
    bool downL = IsMouseButtonDown(MOUSE_BUTTON_LEFT);
    bool downR = IsMouseButtonDown(MOUSE_BUTTON_RIGHT);
    bool pressL = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    bool pressR = IsMouseButtonPressed(MOUSE_BUTTON_RIGHT);

    //Reset drag tracking when no buttons are held
    if (!downL && !downR)
    {
        lastX = lastY = lastZ = -1;
        lastBtn = -1;
        dragLayer = -1;
    }



    //Raycast from mouse to detect hovered voxel
    Ray ray = GetMouseRay(mousePos, cam);
    int  xPlacementPos = -1;
    int yPlacementPos = -1;
    int zPlacementPos = -1;
    float best = FLT_MAX;

    Vector3 normal{ 0,0,0 };
    const Map& M = maps[currentMapIndex];
    for (int l = 0; l < M.levels; ++l)
        for (int z = 0; z < M.height; ++z)
            for (int x = 0; x < M.width; ++x)
            {
                if (ColorEq(M.voxels[coord3dToIndex(l, z, x)], BLACK)) continue; // Skip empty


                //Create a placement bounding box for where were trying to place a block
                BoundingBox box{ {(float)x,(float)l,(float)z}, {(float)x + 1,(float)l + 1,(float)z + 1} };
                //Get the raycast collision from tha box
                auto rayCollision = GetRayCollisionBox(ray, box);

                //If we hit that bounding box and it is as far away as possible then this is a valid placement
                if (rayCollision.hit && rayCollision.distance < best)
                {
                    best = rayCollision.distance;//Best voxel possible
                    xPlacementPos = x; yPlacementPos = l; zPlacementPos = z;//Best hit coordinates
                    normal = rayCollision.normal;//Surface normal
                }
            }


    //Lock layer for placement while holding left click (does this for placement not erasing)
    if (pressL && dragLayer < 0)
    {
        if (xPlacementPos != -1) dragLayer = yPlacementPos + int(roundf(normal.y));//Place on adjacent layer if clicking a block
        else dragLayer = currentLevel;//Else default to current level
    }

    //Decide add/delete mode and brush type
    enum Mode { NONE, SINGLE, BRUSH } mode = NONE;
    int targetX = -1, targetY = -1, targetZ = -1;
    bool wantAdd = false, wantDel = false;
    auto setAdd = [&](bool c) { if (c) { wantAdd = true; mode = (mode == NONE ? SINGLE : mode); } };
    auto setDel = [&](bool c) { if (c) { wantDel = true; mode = (mode == NONE ? SINGLE : mode); } };

    if (xPlacementPos != -1)
    {
        //Determine add/delete based on shift key and button presses
        if (shift)//Single
        {
            setAdd(pressL);
            setDel(pressR);
        }
        else//Brush
        {
            mode = BRUSH;
            wantAdd = downL;
            wantDel = downR;
        }
        //
        if (wantAdd)
        {
            //Add block adjacent to clicked face
            targetX = xPlacementPos + int(roundf(normal.x));
            targetY = dragLayer;
            targetZ = zPlacementPos + int(roundf(normal.z));
        }
        else if (wantDel)
        {
            //Delete block at clicked position
            targetX = xPlacementPos;
            targetY = yPlacementPos;
            targetZ = zPlacementPos;
        }
    }
    else
    {
        //Ray did not hit any blocks, project onto the horizontal plane of current layer instead
        float rayDirectionY = ray.direction.y;
        if (fabs(rayDirectionY) > 1e-4f)
        {
            //Get distance along the ray
            float rayDistance = ((wantAdd ? dragLayer : currentLevel) - ray.position.y) / rayDirectionY;
            if (rayDistance >= 0)
            {
                //Get the 3D space cords of hitpoint
                Vector3 realSpacePosition = Vector3Add(ray.position, Vector3Scale(ray.direction, rayDistance));
                targetX = int(floorf(realSpacePosition.x));
                targetY = wantAdd ? dragLayer : currentLevel;
                targetZ = int(floorf(realSpacePosition.z));

                //Decide what to do with the clicked on position (place or delete)
                if (shift) { setAdd(pressL); setDel(pressR); }
                else { mode = BRUSH; wantAdd = downL; wantDel = downR; }
            }
        }
    }

    //Apply edits based on mode and destination
    auto& voxels = maps[currentMapIndex].voxels;
    auto applyEdit = [&](int editIndex, Color tempVoxels)
        {
            if (ColorEq(voxels[editIndex], tempVoxels)) return; // Skip if same color
            // Record undo data: index and color change
            undoStack.push_back({ editIndex,voxels[editIndex],tempVoxels });//Add to undo stack
            if (undoStack.size() > maxEditHistory) undoStack.erase(undoStack.begin());//If undo stack too big, cut from the bottom
            redoStack.clear();//Clear redo on new edit
            voxels[editIndex] = tempVoxels;//Apply new color
        };

    //Check bounds and perform edit
    if ((wantAdd || wantDel) && inside(targetY, targetZ, targetX))
    {
        if (pressL || pressR)
        {
            //Mark start of placement group for undo
            undoStack.push_back({ placementGroupMarker,BLACK,BLACK });
            if (undoStack.size() > maxEditHistory) undoStack.erase(undoStack.begin());
            redoStack.clear();
        }

        //Determine if continuing same brush stroke
        int cmpX = (xPlacementPos == -1 ? targetX : xPlacementPos);
        int cmpY = (xPlacementPos == -1 ? targetY : yPlacementPos);
        int cmpZ = (xPlacementPos == -1 ? targetZ : zPlacementPos);
        bool same = (cmpX == lastX && cmpY == lastY && cmpZ == lastZ);

        if (mode == SINGLE || !same)
        {
            if (wantAdd) applyEdit(coord3dToIndex(targetY, targetZ, targetX), BlockToPixel(current));
            if (wantDel) applyEdit(coord3dToIndex(targetY, targetZ, targetX), BLACK);
            lastX = cmpX; lastY = cmpY; lastZ = cmpZ;
        }
    }



    /// Hotkeys and commands handling
    //Save and load maps
    if (IsKeyPressed(KEY_S) && IsKeyDown(KEY_LEFT_CONTROL)) SaveMap(currentMapIndex);
    if (IsKeyPressed(KEY_L) && IsKeyDown(KEY_LEFT_CONTROL)) LoadMap(currentMapIndex);

    //Undo and redo
    if (IsKeyPressed(KEY_Z) && IsKeyDown(KEY_LEFT_CONTROL)) undo();
    if (IsKeyPressed(KEY_Y) && IsKeyDown(KEY_LEFT_CONTROL)) redo();

    //Create new blank map
    if (IsKeyPressed(KEY_N) && IsKeyDown(KEY_LEFT_CONTROL)) NewBlank(100, 100, 32);

    //Switch map
    if (IsKeyPressed(KEY_TAB) && IsKeyDown(KEY_LEFT_CONTROL))
    {
        //Switch back if shift is also pressed, else foward
        bool back = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);

        int totalMaps = (int)maps.size();
        SwitchToMap(back ? (currentMapIndex - 1 + totalMaps) % totalMaps : (currentMapIndex + 1) % totalMaps);
    }


    //Clear current layer
    if (IsKeyPressed(KEY_C) && IsKeyDown(KEY_LEFT_CONTROL) && IsKeyDown(KEY_LEFT_SHIFT)) ClearCurrentLayer();
    //Clear all voxels
    if (IsKeyPressed(KEY_C) && IsKeyDown(KEY_LEFT_CONTROL) && !IsKeyDown(KEY_LEFT_SHIFT)) ClearTiles();

}




/// /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Tile Selection Palette
//


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//INIT - Load the tile textures for the palette
void WorldEditor::initPaletteTextures()
{
    //Loads all the palette textures
    //TextureWall = LoadTexture("resources/World/Wall.png");
    //TextureDoor = LoadTexture("resources/World/Door.png");
    //TextureFloor = LoadTexture("resources/World/Floor.png");
    //TextureStone = LoadTexture("resources/World/Stone.png");
    //TEST_TEXTURE = LoadTexture("resources/World/Test.png");
    ////
    //TextureSpawn = LoadTexture("resources/World/Spawn.png");
}

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//Builds out the palette list and toggle button
void WorldEditor::InitPalette()
{

    //Set the palette toggle button position and size
    int buttonWidth = 96;//Palette button width
    int padTiles = 24;//Width between separate tiles
    int pad = 12;//Padding for a tiles position against screen margin
    int screenW = GetScreenWidth();


    //Define the toggle button area for the palette
    paletteToggle = { float(screenW - (buttonWidth + pad + 100)), float(pad), buttonWidth + 103.0f, buttonWidth / 2.0f };
    if (!paletteExpanded) return;//Stop here unless the palette is expanded



    //Define the palette area
    float x = screenW - (buttonWidth + pad);
    float y = padTiles + paletteToggle.height + pad;

    //Helper lambda to add blocks to palette
    auto add = [&](Block b, Texture2D texture, const char* name) { palette.push_back({ b, texture, name }); y += buttonWidth + padTiles; };

    //Add each tile type to the palette
    add(BLK_WALL, TextureWall, "Wall");
    add(BLK_DOOR1, TextureDoor, "Door 1");
    add(BLK_DOOR2, TextureDoor, "Door 2");
    add(BLK_FLOOR, TextureFloor, "Floor");
    add(BLK_STONE, TextureStone, "Stone");
    add(BLK_TEST, TEST_TEXTURE, "Test");

    add(BLK_SPAWN, TextureSpawn, "Spawn");

    add(BLK_STAIR_N, TEST_TEXTURE, "Stair N");
    add(BLK_STAIR_S, TEST_TEXTURE, "Stair S");
    add(BLK_STAIR_E, TEST_TEXTURE, "Stair E");
    add(BLK_STAIR_W, TEST_TEXTURE, "Stair W");

    //Brick
    add(BLK_BRICK1, TextureBrick1, "Brick 1");
    add(BLK_BRICK2, TextureBrick2, "Brick 2");
    add(BLK_BRICK3, TextureBrick3, "Brick 3");
    add(BLK_BRICK4, TextureBrick4, "Brick 4");

    //Brick Faces
    add(BLK_BRICK_FACE1, TextureBrickFace1, "Brick Face 1");
    add(BLK_BRICK_FACE2, TextureBrickFace2, "Brick Face 2");
    add(BLK_BRICK_FACE2_BLOOD, TextureBrickFace2Blood, "Brick Face 2 (Blood)");
    add(BLK_BRICK_FACE3, TextureBrickFace3, "Brick Face 3");
    add(BLK_BRICK_FACE3_BLOOD, TextureBrickFace3Blood, "Brick Face 3 (Blood)");
    add(BLK_BRICK_FACE4, TextureBrickFace4, "Brick Face 4");
    add(BLK_BRICK_FACE5, TextureBrickFace5, "Brick Face 5");
    add(BLK_RED_BRICK, TextureRedBrickTexture, "Red Brick");

    //Cement
    add(BLK_CEMENT_WALL, TextureCementWall, "Cement Wall");
    add(BLK_CEMENT_BOTTOM1, TextureCementWallBottom1, "Cement Bottom 1");
    add(BLK_CEMENT_BOTTOM2, TextureCementWallBottom2, "Cement Bottom 2");
    add(BLK_CEMENT_BOTTOM3, TextureCementWallBottom3, "Cement Bottom 3");
    add(BLK_CEMENT_DECO1, TextureCementWallDeco1, "Cement Deco 1");
    add(BLK_CEMENT_DECO2, TextureCementWallDeco2, "Cement Deco 2");
    add(BLK_CEMENT_TOP1, TextureCementWallTop1, "Cement Top 1");
    add(BLK_CEMENT_TOP2, TextureCementWallTop2, "Cement Top 2");

    //Dark Steel
    add(BLK_DARK_STEEL, TextureDarkSteel, "Dark Steel");
    add(BLK_DARK_STEEL_BEAM, TextureDarkSteelBeam, "Dark Steel Beam");
    add(BLK_DARK_STEEL_DECO, TextureDarkSteelDeco, "Dark Steel Deco");
    add(BLK_DARK_STEEL_HAZARD, TextureDarkSteelHazard, "Dark Steel Hazard");
    add(BLK_DARK_STEEL_TOP1, TextureDarkSteelTop1, "Dark Steel Top 1");
    add(BLK_DARK_STEEL_TOP2, TextureDarkSteelTop2, "Dark Steel Top 2");
    add(BLK_DARK_STEEL_WALL, TextureDarkSteelWall, "Dark Steel Wall");
    add(BLK_DARK_STEEL_WALL_DECO, TextureDarkSteelWallDeco, "Dark Steel Wall Deco");

    //Steel
    add(BLK_OLD_STEEL, TextureOldSteel, "Old Steel");
    add(BLK_RUST_BEAM_UP, TextureRustBeamUp, "Rust Beam Up");
    add(BLK_RUST_BEAM_SIDE, TextureRustBeamSide, "Rust Beam Side");
    add(BLK_STEEL_BEAM1, TextureSteelBeam1, "Steel Beam 1");
    add(BLK_STEEL_BEAM2, TextureSteelBeam2, "Steel Beam 2");
    add(BLK_STEEL_DOOR, TextureSteelDoor, "Steel Door");
    add(BLK_STEEL_GRIP1, TextureSteelGrip1, "Steel Grip 1");
    add(BLK_STEEL_GRIP2, TextureSteelGrip2, "Steel Grip 2");
    add(BLK_STEEL_PLATE, TextureSteelPlate, "Steel Plate");
    add(BLK_STEEL_REINFORCED, TextureSteelReinforced, "Steel Reinforced");

    //Grates
    add(BLK_GRATE, TextureGrate, "Grate");
    add(BLK_GRATE_RUST, TextureGrateRust, "Grate Rust");

    //Misc
    add(BLK_SUPPORT_BEAM, TextureSupportBeam, "Support Beam");
    add(BLK_SWITCH_GREEN, TextureSwitchGreen, "Switch (Green)");
    add(BLK_SWITCH_RED, TextureSwitchRed, "Switch (Red)");
    add(BLK_BIG_DOOR_LEFT1, TextureBigDoorLeft, "Big Door Left 1");//
    add(BLK_BIG_DOOR_RIGHT1, TextureBigDoorRight, "Big Door Right 1");
    add(BLK_BIG_DOOR_LEFT2, TextureBigDoorLeft, "Big Door Left 2");//
    add(BLK_BIG_DOOR_RIGHT2, TextureBigDoorRight, "Big Door Right 2");
    add(BLK_FENCE, TextureFence, "Fence");
    add(BLK_VENT, TextureVent, "Vent");
}

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//Builds the palette scrollbar and updates its position
void WorldEditor::UpdatePaletteScroll(Vector2& mousePos, float wheel)
{
    //Palette scrollbar and handle vars
    float buttonWidth = paletteToggle.width - 20.0f;
    float pad = 12.0f;
    float xPos = paletteToggle.x + 10.0f;
    float yPos = paletteToggle.y + paletteToggle.height + pad;
    float viewportHeight = GetScreenHeight() - yPos - pad;
    float totalHeight = float(palette.size()) * (buttonWidth + pad) - pad;


    //Adjust the scroll position based on mouse wheel input
    if (wheel) paletteScroll = std::clamp(paletteScroll - wheel * (buttonWidth + pad), 0.0f, std::max(0.0f, totalHeight - viewportHeight));


    //Set scrollbar area
    float barWidth = 8.0f;
    paletteScrollBar = { xPos + buttonWidth + 4, yPos, barWidth, viewportHeight };

    if (totalHeight > viewportHeight)
    {
        //Set scrollbar handle height porportunally to visible area
        float handleHeight = viewportHeight * (viewportHeight / totalHeight);
        float verticalOffset = (paletteScroll / (totalHeight - viewportHeight)) * (viewportHeight - handleHeight);
        paletteScrollHandle = { paletteScrollBar.x, yPos + verticalOffset, barWidth, handleHeight };
    }
    else
    {
        //If all tiles fit on screen just set the scrollbar to the same size as the palette (Not moveable)
        paletteScrollHandle = paletteScrollBar;
    }


    //Handle clicking on the scrollbar
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(mousePos, paletteScrollHandle))
    {
        paletteScrolling = true;
    }
    //Set the scroll position based on the mouse position
    if (paletteScrolling && IsMouseButtonDown(MOUSE_BUTTON_LEFT))
    {
        float relativePos = std::clamp(mousePos.y - yPos - paletteScrollHandle.height / 2, 0.0f, paletteScrollBar.height - paletteScrollHandle.height);
        paletteScroll = (relativePos / (paletteScrollBar.height - paletteScrollHandle.height)) * (totalHeight - viewportHeight);
    }
    //Let go of the scrollbar
    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
    {
        paletteScrolling = false;
    }
}

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//Draws the palette toggle, palette tiles and scrollbar
void WorldEditor::DrawPalette()
{
    //Draw toggle button
    DrawRectangleRec(paletteToggle, DARKGRAY);//Background
    DrawRectangleLinesEx(paletteToggle, 2, BLACK);//Outline

    //Text for the toggle button
    const char* toggleText = paletteExpanded ? "Close" : "Tile Picker";
    Vector2 textSize = MeasureTextEx(GetFontDefault(), toggleText, 16, 1);
    //Draw toggle text centered
    DrawText(toggleText, int(paletteToggle.x + (paletteToggle.width - textSize.x) / 2), int(paletteToggle.y + (paletteToggle.height - textSize.y) / 2), 16, BLACK);


    if (!paletteExpanded) return;//Stop here if the palette is not expanded
    //If we get this far we should draw the palette tiles

    //Set the palette area
    const float paletteTileSize = paletteToggle.width - 20.0f;
    const float pad = 12.0f;
    const float xPos = paletteToggle.x + 10.0f;
    const float yPos = paletteToggle.y + paletteToggle.height + pad;
    const float viewportHeight = GetScreenHeight() - yPos - pad;

    //Restrict drawing to the palette area
    BeginScissorMode(int(xPos), int(yPos), int(paletteTileSize), int(viewportHeight));
    for (int i = 0; i < (int)palette.size(); ++i)
    {
        Rectangle destination = { xPos, yPos + i * (paletteTileSize + pad) - paletteScroll, paletteTileSize, paletteTileSize };

        //Draw tile texture
        DrawTexturePro(palette[i].texture, { 0,0,float(palette[i].texture.width),float(palette[i].texture.height) }, destination, { 0,0 }, 0, WHITE);
        //Highlight if selected
        if (palette[i].type == current) DrawRectangleLinesEx(destination, 4, RED);
        //Draw tile name
        DrawText(palette[i].name, int(destination.x + 5), int(destination.y + 5), 28, PURPLE);
    }
    EndScissorMode();

    //Draw scrollbar
    DrawRectangleRec(paletteScrollBar, GRAY);
    DrawRectangleRec(paletteScrollHandle, DARKGRAY);
}




/// /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Commands
//

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//INIT - Build the list of command buttons
void WorldEditor::InitCommands()
{
    //Clear existing command buttons
    commandButtons.clear();
    //Command buttons sizes and positions
    float buttonWidth = 160;
    float buttonHeight = 24;
    float xPos = 10;
    float yPos = 75;
    float pad = 6;
    float gap = pad * 5;

    //Text labels for command buttons
    const char* labels[] = {
        "Save","Load","Undo","Redo","Clear Layer",
        "Clear All","Next Map","Prev Map","New Map"
    };
    //Command IDs for each button
    CmdID ids[] = {
        CMD_Save, CMD_Load, CMD_Undo, CMD_Redo, CMD_ClearLayer,
        CMD_ClearTiles, CMD_NextMap, CMD_PrevMap, CMD_NewMap
    };


    //Local Y position for each command button
    float localYPos = yPos;
    //Creates a button for each command label with its label and ID
    for (int i = 0; i < 9; ++i)
    {
        commandButtons.push_back({ {xPos,localYPos,buttonWidth,buttonHeight}, labels[i], ids[i] });
        localYPos += buttonHeight + ((i == 1 || i == 5) ? gap : pad);
    }

    //Define toggle area for commands palette (prevent clicks behind it)
    cmdsPaletteToggle = { xPos, yPos - (buttonHeight + pad), buttonWidth, buttonHeight };
}

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//Handle clicking on command buttons
bool WorldEditor::CheckCommandClicks()
{
    //Return false if not left clicking
    if (!IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) return false;


    //Get mouse pos
    Vector2 mousePos = GetMousePosition();

    //Check each button rect for collision
    for (auto& button : commandButtons)
    {
        if (CheckCollisionPointRec(mousePos, button.rect))
        {
            DoCommand(button.id);
            return true;
        }
    }
    return false;
}

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//Draws the command palette toggle button and if expanded also calls for the drawing of the command buttons
void WorldEditor::DrawCommandPalette()
{
    //Draw commands toggle button
    DrawRectangleRec(cmdsPaletteToggle, DARKGRAY);//Background
    DrawRectangleLinesEx(cmdsPaletteToggle, 2, BLACK);//Outline

    //Command toggle text
    const char* name = cmdsPaletteExpanded ? "Close Commands" : "Open Commands";
    Vector2 textSize = MeasureTextEx(GetFontDefault(), name, 16, 1);
    DrawText(name, int(cmdsPaletteToggle.x + (cmdsPaletteToggle.width - textSize.x) / 2), int(cmdsPaletteToggle.y + (cmdsPaletteToggle.height - textSize.y) / 2), 16, BLACK);

    //Draw all the command buttons if expanded
    if (cmdsPaletteExpanded) DrawCommands();
}
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//Draws command buttons boxes and their text (if expanded in the above function)
void WorldEditor::DrawCommands()
{
    //Loop through and draw all the command buttons
    for (auto& buttons : commandButtons)
    {
        //Draw button
        DrawRectangleRec(buttons.rect, LIGHTGRAY);//Background
        DrawRectangleLinesEx(buttons.rect, 2, DARKGRAY);//Outline
        //Draw text
        Vector2 textSize = MeasureTextEx(GetFontDefault(), buttons.label, 20, 1);
        DrawText(buttons.label, int(buttons.rect.x + (buttons.rect.width - textSize.x) / 2), int(buttons.rect.y + (buttons.rect.height - textSize.y) / 2), 20, BLACK);
    }
}

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//Calls a function to execute a command based on the button pressed
void WorldEditor::DoCommand(CmdID id)
{
    //Command list with their ids and function
    switch (id)
    {
    case CMD_Save: SaveMap(currentMapIndex); break;
    case CMD_Load: LoadMap(currentMapIndex); currentLevel = 0; break;

    case CMD_Undo: undo(); break;
    case CMD_Redo: redo(); break;

    case CMD_ClearLayer: ClearCurrentLayer(); break;
    case CMD_ClearTiles: ClearTiles(); break;

    case CMD_NextMap: SwitchToMap((currentMapIndex + 1) % maps.size()); break;
    case CMD_PrevMap: SwitchToMap((currentMapIndex - 1 + maps.size()) % maps.size()); break;
    case CMD_NewMap: NewBlank(maps[currentMapIndex].width, maps[currentMapIndex].height, maps[currentMapIndex].levels); break;
    }
}

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//Save the current map selected to a png file in the maps folder
bool WorldEditor::SaveMap(int index) const
{
    //Get map at current index
    const Map& map = maps[index];

    //Ensure index and map dimensions are within expected limits
    if (index < 0 || index >= (int)maps.size()) return false;
    if (map.width <= 0 || map.height <= 0 || map.levels <= 0) return false;

    //Create image of map size and populate it with empty black pixels
    Image img = GenImageColor(map.width, map.height * map.levels, BLACK);
    //Write to memory saved pixel data into the img data
    std::memcpy(img.data, map.voxels.data(), sizeof(Color) * size_t(map.width) * map.height * map.levels);

    //Try export image with map data to its file path, unload then return if it was successful
    bool saved = ExportImage(img, mapPaths[index].c_str());
    UnloadImage(img);
    return saved;
}

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//Load the current map selected from its png file in the maps folder, or return false if not found
bool WorldEditor::LoadMap(int index)
{
    //Get map path for the map at the index we are looking for
    const char* path = mapPaths[index].c_str();
    //Make sure its within bounds
    if (index < 0 || index >= (int)mapPaths.size()) return false;


    //Load the image
    Image loadedImg = LoadImage(path);
    if (!loadedImg.data) return false;//Failed to load return false


    //Build a temporary map from the image
    Map loaded;
    loaded.width = loadedImg.width;
    loaded.height = loadedImg.height / MAP_LEVELS;
    loaded.levels = MAP_LEVELS;
    loaded.voxels.resize(size_t(loaded.width) * loaded.height * loaded.levels);

    //Copy pixel data into voxel storage
    Color* src = LoadImageColors(loadedImg);
    std::memcpy(loaded.voxels.data(), src, sizeof(Color) * loaded.width * loaded.height * loaded.levels);
    //Unload image now that we have stored voxel data
    UnloadImageColors(src);
    UnloadImage(loadedImg);

    //Replaces current map with the loaded one, returns success
    maps[index] = std::move(loaded);
    return true;
}

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//Undo the last edit group
void WorldEditor::undo()
{
    //Stack is empty, dont continue, nothing to do
    if (undoStack.empty()) return;

    //Pop edits until we hit a group marker
    while (!undoStack.empty())
    {
        Edit edit = undoStack.back(); undoStack.pop_back();//Fetch top edit off the stack
        redoStack.push_back(edit);//Push this edit onto the redo stack so we can redo the undo later if wanted
        if (edit.index == placementGroupMarker) break;//Stop undoing if we reach the start of a new group

        //Revert previous voxel color at this index
        maps[currentMapIndex].voxels[edit.index] = edit.before;
    }
}
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//Redo the last edit group
void WorldEditor::redo()
{
    //Stack is empty, dont continue, nothing to redo
    if (redoStack.empty()) return;

    //Pop redo edits until we hit a group marker
    while (!redoStack.empty())
    {
        Edit edit = redoStack.back(); redoStack.pop_back();//Fetch top edit off the stack
        undoStack.push_back(edit);//Push this edit onto the undo stack so we can undo the redo later if wanted
        if (edit.index == placementGroupMarker) break;//Stop redoing if we reach the start of a new group

        //Revert previous voxel color at this index
        maps[currentMapIndex].voxels[edit.index] = edit.after;
    }
}

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//Clears all voxels in the current layer
void WorldEditor::ClearCurrentLayer()
{
    //Only do if the map isnt empty
    if (maps.empty()) return;

    //Mark start of clear action for undo
    undoStack.push_back({ placementGroupMarker,BLACK,BLACK });

    //Vars
    int width = maps[currentMapIndex].width;
    int height = maps[currentMapIndex].height;
    //Loop through the voxels on the current level and clear
    for (int y = 0; y < height; ++y)
        for (int x = 0; x < width; ++x)
        {
            int index = coord3dToIndex(currentLevel, y, x);
            if (!ColorEq(maps[currentMapIndex].voxels[index], BLACK))
            {
                // Record undo and clear voxel
                undoStack.push_back({ index, maps[currentMapIndex].voxels[index], BLACK });
                maps[currentMapIndex].voxels[index] = BLACK;
                if (undoStack.size() > maxEditHistory) undoStack.erase(undoStack.begin());
            }
        }

    redoStack.clear();//Clear redo history
    lastX = lastY = lastZ = -1;//Reset drag tracking
    lastBtn = -1;
}
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//Clears all voxels in current map
void WorldEditor::ClearTiles()
{
    //Mark clear all edit for undo
    undoStack.push_back({ placementGroupMarker,BLACK,BLACK });


    //Loop through every voxel
    for (size_t i = 0; i < maps[currentMapIndex].voxels.size(); ++i)
    {
        //If the voxel isnt empty empty it
        if (!ColorEq(maps[currentMapIndex].voxels[i], BLACK))
        {
            //Record undo and empty the voxel black
            undoStack.push_back({ int(i), maps[currentMapIndex].voxels[i], BLACK });//Record to undo stack
            maps[currentMapIndex].voxels[i] = BLACK;//Convert to empty voxel
            if (undoStack.size() > maxEditHistory) undoStack.erase(undoStack.begin());//If there are too many edits clear the stack from the bottom
        }
    }

    //Clear redo stack
    redoStack.clear();
    //Rest dragging and which button was pressed
    lastX = lastY = lastZ = -1;
    lastBtn = -1;
}

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//Switch back and forth between maps, saving the current one when switching and the loading the new one
void WorldEditor::SwitchToMap(int newIndex)
{
    //Make sure the map were trying to get to exists
    if (newIndex < 0 || newIndex >= (int)maps.size()) return;

    //Save current map before switching
    SaveMap(currentMapIndex);

    //Move to new map
    currentMapIndex = newIndex;
    currentLevel = 0;//Reset edit layer

    //Load new map or create an empty one if failed
    if (!LoadMap(currentMapIndex)) maps[currentMapIndex].voxels.assign(size_t(MAP_WIDTH) * MAP_LENGHT * MAP_LEVELS, BLACK);

    //Reset palette for new map
    InitPalette();
}

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//Create a new blank map with the given width, height and levels (will not save unless done manually)
void WorldEditor::NewBlank(int w, int h, int lvls)
{
    Map map;
    map.width = w;
    map.height = h;
    map.levels = lvls;

    //Initialize voxels to all black
    map.voxels.assign(w * h * lvls, BLACK);

    //Append new map to map and mapPaths lists
    maps.push_back(std::move(map));
    mapPaths.push_back(DefaultMapPath((int)maps.size() - 1));

    //Refresh build settings and palette
    currentLevel = 0;
    lastX = lastY = lastZ = -1;
    InitPalette();
}