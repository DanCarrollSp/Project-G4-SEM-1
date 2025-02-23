#include "raylib.h"
#include "Enemy.h"

Enemy::Enemy() 
{
    position = { 5.0f, 0.5f, 5.0f };
    speed = 2.0f;

    // Set up an initial hitbox (assuming a 1x1x1 size enemy)
    hitbox.min = { position.x - hitBoxWidth, position.y - hitBoxHeight, position.z - hitBoxWidth };
    hitbox.max = { position.x + hitBoxWidth, position.y + hitBoxHeight, position.z + hitBoxWidth };
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