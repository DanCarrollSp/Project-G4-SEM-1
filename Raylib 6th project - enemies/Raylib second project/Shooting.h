#ifndef SHOOTING_H
#define SHOOTING_H

//Raylib
#include "raylib.h"
#include "raymath.h"
//Libs
#include <vector>
#include <float.h>//FLT_MAX
#include <cmath>
//Local Libs
#include "Enemy.h"
#include "ParticleSystem.h"
#include "Decal.h"



//Hit types for obsticles and entities
enum HitType 
{
    HIT_NONE = 0,
    HIT_WALL,
    HIT_DOOR,
    HIT_ENEMY
};

// Processes a bullet shot: casts a ray from the camera, if something is hit:
//applies the proper effect (adds a decal, spawns blood particles, and/or damages an enemy)
void ProcessBulletShot(Camera& camera,
    const std::vector<BoundingBox>& wallBoxes,
    const std::vector<BoundingBox>& doorBoxes,
    std::vector<Enemy>& enemies);


//Stuff that always happens when you shoot, like ejecting a shell casings
void alwaysOnShot();


//Crosshair color changes to red if an enemy is in sight and not blocked by obsticles
bool IsEnemyInSight(Camera& camera,
    const std::vector<BoundingBox>& wallBoxes,
    const std::vector<BoundingBox>& doorBoxes,
    std::vector<Enemy>& enemies);

#endif // SHOOTING_H