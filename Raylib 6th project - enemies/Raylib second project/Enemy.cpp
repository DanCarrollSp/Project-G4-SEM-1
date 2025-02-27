#include "raylib.h"
#include "Enemy.h"
#include <raymath.h>
#include "Pathfinding.h"

Enemy::Enemy() 
{
    position = { 5.0f, 0.5f, 5.0f };
    speed = 2.0f;



    currentPathIndex = 0;
    currentPath.clear();



    // Set up an initial hitbox
    hitbox.min = { position.x - hitBoxWidth, position.y - hitBoxHeight, position.z - hitBoxWidth };
    hitbox.max = { position.x + hitBoxWidth, position.y + hitBoxHeight, position.z + hitBoxWidth };

    timeSinceLastPathRecalc = 0.0f;
}

Enemy::~Enemy() 
{

}

void Enemy::Update()
{
    //Temp movment
    if (IsKeyDown(KEY_RIGHT)) position.x += speed * GetFrameTime();
    if (IsKeyDown(KEY_LEFT)) position.x -= speed * GetFrameTime();
    if (IsKeyDown(KEY_UP)) position.z -= speed * GetFrameTime();
    if (IsKeyDown(KEY_DOWN)) position.z += speed * GetFrameTime();

    // Update hitbox based on new position
    hitbox.min = { position.x - hitBoxWidth, position.y - hitBoxHeight, position.z - hitBoxWidth };
    hitbox.max = { position.x + hitBoxWidth, position.y + hitBoxHeight, position.z + hitBoxWidth };

    animate();
}


void Enemy::Draw(Camera camera)
{
    DrawBillboard(camera, currentTexture, position, 1.0f, WHITE);
    DrawBoundingBox(hitbox, RED);
}

void Enemy::animate()
{
    static double lastUpdateTime = 0.0; // Tracks the last time the texture was updated
    static int frameIndex = 0;          // Index of the current walk texture

    if (!texturesLoaded)
    {
        // Load walk textures only once
        walkTextures[0] = LoadTexture("resources/enemies/Walk1.png");
        walkTextures[1] = LoadTexture("resources/enemies/Walk2.png");
        walkTextures[2] = LoadTexture("resources/enemies/Walk3.png");
        walkTextures[3] = LoadTexture("resources/enemies/Walk4.png");

        texturesLoaded = true;
    }

    double currentTime = GetTime();
    if (currentTime - lastUpdateTime >= 0.15) // Check if 1 second has passed
    {
        lastUpdateTime = currentTime;
        frameIndex = (frameIndex + 1) % 4; // Cycle through textures
        currentTexture = walkTextures[frameIndex];       // Set current texture
    }
}

Vector3 Enemy::collision(Ray ray)
{
    // Check if the ray hits the enemy's hitbox
    RayCollision collision = GetRayCollisionBox(ray, hitbox);
    if (collision.hit)
    {
        Vector3 collisionPoint = collision.point;
        // Handle collision logic here
        return collisionPoint;
    }
    return {0,0,0};
}

void Enemy::Move(Vector3 target, const std::vector<std::vector<bool>>& navGrid, float deltaTime)
{
    //Only move if not at the target position
    if (Vector3Distance(position, target) <= 0.5f)
    {
        return;
    }

    //deltaTime for path recalculation
    timeSinceLastPathRecalc += deltaTime;
    //Recalculates the path if 1 second has passed
    if (timeSinceLastPathRecalc >= 1.0f)
    {
        RecalculatePath(target, navGrid);
        timeSinceLastPathRecalc = 0.0f;
    }

    

    //If a path was found, follow it
    if (!currentPath.empty() && currentPathIndex < (int)currentPath.size())
    {
        Vector3 nextWaypoint = currentPath[currentPathIndex];

        //Gets direction
        Vector3 dir = {
            nextWaypoint.x - position.x,
            0.0f, //Y not used
            nextWaypoint.z - position.z
        };


        //Calculates the distance to next waypoint
        float dist = Vector3Length(dir);
        //Normalize di
        if (dist > 0.01f) dir = Vector3Scale(dir, 1.0f / dist);


        //Move enemy by speed * dt
        float step = speed * deltaTime;
        if (dist > step)
        {
            //Move
            position.x += dir.x * step;
            position.z += dir.z * step;
        }
        else
        {
            //Reach (or pass) the next waypoint this frame
            position = nextWaypoint;
            currentPathIndex++;
        }
    }
    else
    {
        //if path is finished or empty
        //
    }
}

void Enemy::RecalculatePath(Vector3 target, const std::vector<std::vector<bool>>& navGrid)
{
    //Compute a new path from current position to target
    currentPath = AStarPath(position, target, navGrid);
    currentPathIndex = 0;//start
}
