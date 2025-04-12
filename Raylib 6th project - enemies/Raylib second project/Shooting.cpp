#include "Shooting.h"


//-------------------------------------------------------------------------- //
//Must be correct varible names from main!
extern DecalManager decalManager;               // Manages decals
extern ParticleSystem particleSystem;           // Handles particle effects
extern ParticleParams bloodParams;              // Parameters for blood effect
extern ParticleParams shellCasingParams;        // Parameters for shell casing effect
extern Texture2D bulletHole;                    // Texture for bullet holes
//-------------------------------------------------------------------------- //



void ProcessBulletShot(
    Camera& camera,
    const std::vector<BoundingBox>& wallBoxes,
    const std::vector<BoundingBox>& doorBoxes,
    std::vector<Enemy>& enemies)
{

	//Stuff we do every time we shoot regardless of what we hit  (e.g. shell casings ejected from gun)
    alwaysOnShot();


    //Creates a ray starting from the camera in the direction it is facing.
    Vector3 rayOrigin = camera.position;
    Vector3 rayDirection = Vector3Normalize(Vector3Subtract(camera.target, camera.position));
    Ray ray = { rayOrigin, rayDirection };

    //Resets vars
    float closestHit = FLT_MAX;
    HitType hitType = HIT_NONE;
    Vector3 hitPoint = { 0, 0, 0 };
    BoundingBox hitBox;  //For normal estimation
    Enemy* hitEnemy = nullptr;


    //Checks for wall collisions
    for (const auto& box : wallBoxes)
    {
        RayCollision collision = GetRayCollisionBox(ray, box);
        if (collision.hit && collision.distance < closestHit)
        {
            closestHit = collision.distance;
            hitType = HIT_WALL;
            hitPoint = collision.point;
            hitBox = box;
            hitEnemy = nullptr;
        }
    }

	//Checks for door collisions
    for (const auto& box : doorBoxes)
    {
        RayCollision collision = GetRayCollisionBox(ray, box);
        if (collision.hit && collision.distance < closestHit)
        {
            closestHit = collision.distance;
            hitType = HIT_DOOR;
            hitPoint = collision.point;
            hitBox = box;
            hitEnemy = nullptr;
        }
    }

	//Checks for enemy collisions (alive)
    for (auto& enemy : enemies)
    {
        if (!enemy.IsAlive()) continue;

        RayCollision collision = GetRayCollisionBox(ray, enemy.hitbox);
        if (collision.hit && collision.distance < closestHit)
        {
            closestHit = collision.distance;
            hitType = HIT_ENEMY;
            hitPoint = collision.point;
            hitBox = enemy.hitbox;
            hitEnemy = &enemy;
        }
    }

    //If nothing was hit, return
    if (hitType == HIT_NONE) return;


	//If we get this far, we know we hit something, process what to do based on what we hit:
    if (hitType == HIT_WALL || hitType == HIT_DOOR)
    {
        //Computes surface normal based on the hit position and bounding box
        Vector3 normal = EstimateNormalFromHit(hitPoint, hitBox);
        //Adds decal to the surface
        decalManager.AddDecal(hitPoint, normal, 0.1f, &bulletHole);
    }
    else if (hitType == HIT_ENEMY)
    {
		//Blood particle effect
        particleSystem.Instantiate(bloodParams);
		//Do damage to the enemy
        if (hitEnemy != nullptr) hitEnemy->TakeDamage(10);
    }
}

void alwaysOnShot()
{
    //particleSystem.Instantiate(shellCasingParams);
}


//Crosshair color changes to red if an enemy is in sight
bool IsEnemyInSight(
    Camera& camera,
    const std::vector<BoundingBox>& wallBoxes,
    const std::vector<BoundingBox>& doorBoxes,
    std::vector<Enemy>& enemies)
{

    Vector3 rayOrigin = camera.position;
    Vector3 rayDirection = Vector3Normalize(Vector3Subtract(camera.target, camera.position));
    Ray ray = { rayOrigin, rayDirection };

    float closestHit = FLT_MAX;
    HitType hitType = HIT_NONE;


    //Checks wall collisions
    for (const auto& box : wallBoxes)
    {
        RayCollision collision = GetRayCollisionBox(ray, box);
        if (collision.hit && collision.distance < closestHit)
        {
            closestHit = collision.distance;
            hitType = HIT_WALL;
        }
    }

    //Checks door collisions
    for (const auto& box : doorBoxes)
    {
        RayCollision collision = GetRayCollisionBox(ray, box);
        if (collision.hit && collision.distance < closestHit)
        {
            closestHit = collision.distance;
            hitType = HIT_DOOR;
        }
    }


    //Checks enemy collisions
    for (auto& enemy : enemies)
    {
        if (!enemy.IsAlive())
            continue;

        RayCollision collision = GetRayCollisionBox(ray, enemy.hitbox);
        if (collision.hit && collision.distance < closestHit)
        {
            closestHit = collision.distance;
            hitType = HIT_ENEMY;
        }
    }

    //Only return true if the first hit is an enemy
    return (hitType == HIT_ENEMY);
}