#include "raylib.h"
#include <stdlib.h>
#include <time.h>

#define MAP_WIDTH 20       // Width of the maze
#define MAP_HEIGHT 20      // Height of the maze
#define CELL_SIZE 1        // Size of each cell in the maze

// Function to generate a random maze with walls and paths
Image GenerateProceduralMap(int width, int height)
{
    // Create an empty image with all pixels set to black
    Image map = GenImageColor(width, height, BLACK);
    Color* pixels = LoadImageColors(map);

    // Simple randomized maze generation
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            // Randomly place walls or paths
            if (rand() % 4 == 0) {
                pixels[y * width + x] = WHITE;  // Wall
            }
            else {
                pixels[y * width + x] = BLACK;  // Path
            }
        }
    }

    // Set start and end points
    pixels[1 * width + 1] = BLACK;                 // Start point
    pixels[(height - 2) * width + (width - 2)] = BLACK;  // End point

    // Apply modified pixel colors back to the image manually
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            ImageDrawPixel(&map, x, y, pixels[y * width + x]);
        }
    }

    UnloadImageColors(pixels);   // Free the color array since it's no longer needed
    return map;
}


int main(void)
{
    const int screenWidth = 1920;
    const int screenHeight = 1080;
    InitWindow(screenWidth, screenHeight, "raylib [models] example - procedural first person maze");

    // Initialize random seed for procedural generation
    srand((unsigned int)time(NULL));

    // Define the camera to look into our 3D world
    Camera camera = { 0 };
    camera.position = Vector3{ 0.2f, 0.4f, 0.2f };    // Camera position
    camera.target = Vector3{ 0.185f, 0.4f, 0.0f };    // Camera looking at point
    camera.up = Vector3{ 0.0f, 10.0f, 0.0f };          // Camera up vector (rotation towards target)
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    // Generate the procedural map
    Image imMap = GenerateProceduralMap(MAP_WIDTH, MAP_HEIGHT);
    Texture2D cubicmap = LoadTextureFromImage(imMap);       // Convert image to texture to display (VRAM)
    Mesh mesh = GenMeshCubicmap(imMap, Vector3{ 1.0f, 1.0f, 1.0f });
    Model model = LoadModelFromMesh(mesh);

    // Load texture atlas (assuming you have a texture atlas for walls/floors)
    Texture2D texture = LoadTexture("resources/cubicmap_atlas.png");
    model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture;

    // Get map image data for collision detection
    Color* mapPixels = LoadImageColors(imMap);
    UnloadImage(imMap);  // Unload procedural image from RAM

    Vector3 mapPosition = { -10.0f, 0.0f, -10.0f };  // Set model position

    DisableCursor();  // Limit cursor to relative movement inside the window
    SetTargetFPS(60); // Set game to run at 60 FPS

    while (!WindowShouldClose())
    {
        Vector3 oldCamPos = camera.position;  // Store old camera position
        UpdateCamera(&camera, CAMERA_FIRST_PERSON);

        // Prevent up and down (pitch) rotation of the camera
        //camera.target.y = 20.0f;  // Set pitch to 0 to stop vertical rotation

        // 2D collision detection
        Vector2 playerPos = { camera.position.x, camera.position.z };
        float playerRadius = 0.1f;

        int playerCellX = (int)(playerPos.x - mapPosition.x + 0.5f);
        int playerCellY = (int)(playerPos.y - mapPosition.z + 0.5f);

        // Check boundaries and map collisions
        if (playerCellX < 0) playerCellX = 0;
        else if (playerCellX >= MAP_WIDTH) playerCellX = MAP_WIDTH - 1;

        if (playerCellY < 0) playerCellY = 0;
        else if (playerCellY >= MAP_HEIGHT) playerCellY = MAP_HEIGHT - 1;

        if (mapPixels[playerCellY * MAP_WIDTH + playerCellX].r == 255)  // Wall collision
        {
            camera.position = oldCamPos;
        }

        BeginDrawing();
        ClearBackground(LIGHTGRAY);

        BeginMode3D(camera);
        DrawModel(model, mapPosition, 1.0f, WHITE);
        EndMode3D();

        DrawTextureEx(cubicmap, Vector2{ screenWidth - MAP_WIDTH * 4.0f - 20, 20.0f }, 0.0f, 4.0f, WHITE);
        DrawRectangleLines(screenWidth - MAP_WIDTH * 4 - 20, 20, MAP_WIDTH * 4, MAP_HEIGHT * 4, RED);
        DrawRectangle(screenWidth - MAP_WIDTH * 4 - 20 + playerCellX * 4, 20 + playerCellY * 4, 4, 4, RED);

        DrawFPS(10, 10);
        EndDrawing();
    }

    UnloadImageColors(mapPixels);   // Free the color array
    UnloadTexture(cubicmap);        // Free the map texture
    UnloadTexture(texture);         // Free the texture atlas
    UnloadModel(model);             // Free the model

    CloseWindow();                  // Close window and OpenGL context

    return 0;
}