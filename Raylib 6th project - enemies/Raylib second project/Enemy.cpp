#include "raylib.h"
#include "Enemy.h"
#include <raymath.h>
#include "Pathfinding.h"

Enemy::Enemy() 
{
    position = { 5.0f, 0.5f, 5.0f };
    lastPosition = position;
    speed = 3.0f;



    currentPathIndex = 0;
    gridPath.clear();



    // Set up an initial hitbox
    hitbox.min = { position.x - hitBoxWidth, position.y - hitBoxHeight, position.z - hitBoxWidth };
    hitbox.max = { position.x + hitBoxWidth, position.y + hitBoxHeight, position.z + hitBoxWidth };

    timeSinceLastPathRecalc = 0.0f;

    //for (int i = 0; i++; i > 4) SetTextureFilter(walkTextures[i], TEXTURE_FILTER_BILINEAR);

    // Initialize health and status
    health = 100;
    isAlive = true;
}

Enemy::~Enemy() 
{

}

void Enemy::SetPosition(const Vector3& newPos)
{
    position = newPos;

    //Updates bounding box
    hitbox.min = { position.x - hitBoxWidth, position.y - hitBoxHeight, position.z - hitBoxWidth };
    hitbox.max = { position.x + hitBoxWidth, position.y + hitBoxHeight, position.z + hitBoxWidth };
}

void Enemy::Update()
{

    //Temp movment
    if (IsKeyDown(KEY_RIGHT)) position.x += speed * GetFrameTime();
    if (IsKeyDown(KEY_LEFT)) position.x -= speed * GetFrameTime();
    if (IsKeyDown(KEY_UP)) position.z -= speed * GetFrameTime();
    if (IsKeyDown(KEY_DOWN)) position.z += speed * GetFrameTime();

    animate();
}


void Enemy::Draw(Camera camera)
{

    DrawBillboard(camera, currentTexture, position, 1.0f, WHITE);
    if (!debug) DrawBoundingBox(hitbox, RED);
}



void Enemy::animate()
{

    if (!texturesLoaded)
    {
        // Load walk textures only once
        walkTextures[0] = LoadTexture("resources/enemies/D1.png");
        walkTextures[1] = LoadTexture("resources/enemies/D2.png");
        walkTextures[2] = LoadTexture("resources/enemies/D3.png");
        walkTextures[3] = LoadTexture("resources/enemies/D4.png");

        // Optionally apply texture filtering here:
        for (int i = 0; i < 4; i++) {
            SetTextureFilter(walkTextures[i], TEXTURE_FILTER_BILINEAR);
        }
        texturesLoaded = true;

        // Set initial texture to idle frame
        currentTexture = walkTextures[0];
    }

    double currentTime = GetTime();
    // Determine if the enemy has moved (using a very small threshold)
    float movementThreshold = 0.001f;
    bool isMoving = Vector3Distance(position, lastPosition) > movementThreshold;

    if (isMoving)
    {
        // Only cycle through animation frames if the enemy is actually moving
        if (currentTime - lastAnimationTime >= 0.125)
        {
            lastAnimationTime = currentTime;
            frameIndex = (frameIndex + 1) % 4; // Cycle through textures
            currentTexture = walkTextures[frameIndex];
        }
    }
    else
    {
        // If the enemy isn’t moving, revert to the idle (first) frame
        frameIndex = 0;
        currentTexture = walkTextures[0];
    }

    // Update lastPosition for the next frame
    lastPosition = position;
}

//Deprecated, not used anymore since handling of shooting done in shooting.cpp (keeping for now in case its late and i feel like ill regret deleting it tomorrow morning)
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

void Enemy::Move(Vector3 target, const std::vector<std::vector<bool>>& navGrid, const std::vector<BoundingBox>& walls, float deltaTime)
{

    //Only move if not at the target position
    if (Vector3Distance(position, target) <= 0.55f && Vector3Distance(position, target) >= -0.55f)
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
    if (!smoothPath.empty() && currentPathIndex < (int)smoothPath.size())
    {
        Vector3 nextWaypoint = smoothPath[currentPathIndex];

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
            position.x += dir.x * step;
            position.z += dir.z * step;
        }
        else
        {
            //Reach (or pass) the next waypoint this frame
            position.x = nextWaypoint.x;
            position.z = nextWaypoint.z;
            currentPathIndex++;
        }
    }
    else
    {
        //if path is finished or empty
        currentPathIndex = 0;
    }

	vectorCollision(walls);
}


void Enemy::RecalculatePath(Vector3 target, const std::vector<std::vector<bool>>& navGrid)
{
    //Compute a new path from current position to target
    gridPath = AStarPath(position, target, navGrid);
    smoothPath = SmoothPath(gridPath, navGrid);
    //currentPathIndex = 0;//start
}


void Enemy::vectorCollision(const std::vector<BoundingBox>& walls)
{
    if (!isAlive) return;

    //Update bounding box based on position
    hitbox.min = { position.x - hitBoxWidth, position.y - hitBoxHeight, position.z - hitBoxWidth };
    hitbox.max = { position.x + hitBoxWidth, position.y + hitBoxHeight, position.z + hitBoxWidth };


    //Checks collision with each wall and resolves it by "pushing" out along the smallest axis overlap
    for (auto& wall : walls)
    {
        if (CheckCollisionBoxes(hitbox, wall))
        {
            //Calculates the overlaps on the X axis
            float overlapX1 = wall.max.x - hitbox.min.x; // overlap from left
            float overlapX2 = hitbox.max.x - wall.min.x;   // overlap from right
            float fixX = (overlapX1 < overlapX2) ? overlapX1 : -overlapX2;

            //Calculates the overlaps on the Z axis
            float overlapZ1 = wall.max.z - hitbox.min.z;
            float overlapZ2 = hitbox.max.z - wall.min.z;
            float fixZ = (overlapZ1 < overlapZ2) ? overlapZ1 : -overlapZ2;

			//Chooses the axis with the smaller penetration and pushes out along that axis (away from the wall, simulating collision physics)
            if (fabsf(fixX) < fabsf(fixZ)) position.x += fixX;
            else position.z += fixZ;

            //Updates the bounding box after resolving the collision
            hitbox.min = { position.x - hitBoxWidth, position.y - hitBoxHeight, position.z - hitBoxWidth };
            hitbox.max = { position.x + hitBoxWidth, position.y + hitBoxHeight, position.z + hitBoxWidth };
        }
    }
}

void Enemy::TakeDamage(int amount)
{
    health -= amount;
    if (health <= 0)
    {
        health = 0;
        isAlive = false;
        // You can play a death animation or sound here
    }
}