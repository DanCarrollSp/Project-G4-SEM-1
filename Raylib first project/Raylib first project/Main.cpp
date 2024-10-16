#include "raylib.h"

int main()
{
    // Initialize window with correct dimensions and title
    InitWindow(800, 500, "My Raylib Window");

    // Main game loop
    while (!WindowShouldClose()) // Detect window close button or ESC key
    {
        // Update and draw code goes here
        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawText("Welcome to Raylib!", 190, 200, 20, LIGHTGRAY);
        EndDrawing();
    }

    // Close the window
    CloseWindow();

    return 0;
}
