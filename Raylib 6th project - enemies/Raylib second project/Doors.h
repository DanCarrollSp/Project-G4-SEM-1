#pragma once

#include "raylib.h"
#include "raymath.h"

#include "Globals.h"
#include "World.h"
#include <vector>


namespace Doors
{
    void Update();
    void Draw();
}

struct DoorConfig
{
    //Everything to build the Doors
    BoundingBox* box;
    Vector3 closedMin, closedMax;
    Vector3 openMin, openMax;
    Texture2D texture;

    //Door status
    bool targetOpen = false;
    float progress = 0.0f;//0 == Closed, 1 == Opened, inbetween == inbetween
};