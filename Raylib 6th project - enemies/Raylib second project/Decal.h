#pragma once

#include "raylib.h"
#include "rlgl.h"
#include <vector>
#include <cmath>

struct Decal 
{
    Vector3 position;
    Vector3 normal;
    float size;
    Texture2D* texture;
    float lifetime;
    float age = 0.0f;

    Decal(Vector3 pos, Vector3 norm, float s, Texture2D* tex, float life)
        : position(pos), normal(norm), size(s), texture(tex), lifetime(life)
    {}
};

class DecalManager 
{
public:
    void AddDecal(Vector3 position, Vector3 normal, float size, Texture2D* texture, float lifetime = 10.0f);
    void Update(float deltaTime);
    void Draw();

private:
    std::vector<Decal> decals;
};

//Estimates surface normal from a bounding box and a hit point on it
Vector3 EstimateNormalFromHit(Vector3 hit, BoundingBox box);
