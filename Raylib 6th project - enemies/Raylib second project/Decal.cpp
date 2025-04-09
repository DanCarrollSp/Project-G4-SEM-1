#include "Decal.h"
#include <raymath.h>


//Adds a new decal
void DecalManager::AddDecal(Vector3 position, Vector3 normal, float size, Texture2D* texture, float lifetime)
{
    //Slightly offset the decal away from the surface to prevent Z fighting
    position = Vector3Add(position, Vector3Scale(normal, 0.01f));

    //Stores the decal in the decal list
    decals.emplace_back(position, normal, size, texture, lifetime);
}


//Updates the state of all decals (fades them out over time)
void DecalManager::Update(float deltaTime)
{
    //Loop through decals in reverse so we can remove them
    for (int i = decals.size() - 1; i >= 0; i--)
    {
        decals[i].age += deltaTime;//Increments the age of the decal

        //If the decal has lived past its lifetime, remove it
        if (decals[i].age >= decals[i].lifetime) decals.erase(decals.begin() + i);
    }
}


//Draws all active decals
void DecalManager::Draw()
{
    for (auto& d : decals)
    {
        //Calculates local axis for the decal quad using the surface normal
        Vector3 right = Vector3Normalize(Vector3CrossProduct(d.normal, Vector3{ 0, 1, 0 }));

        //Fallback: if normal is pointing directly up/down, cross product will fail (zero length vector)
        if (Vector3Length(right) < 0.001f) right = Vector3{ 1, 0, 0 };//fallback right direction

        Vector3 up = Vector3Normalize(Vector3CrossProduct(right, d.normal));//up vector based on right and normal

        float halfSize = d.size / 2;//Half size of decal
        Vector3 center = d.position;//Center of decal

        //Calculates the 4 corners of the quad using center + right + up vectors
        Vector3 p1 = Vector3Add(Vector3Subtract(center, Vector3Scale(right, halfSize)), Vector3Scale(up, halfSize));
        Vector3 p2 = Vector3Add(Vector3Add(center, Vector3Scale(right, halfSize)), Vector3Scale(up, halfSize));
        Vector3 p3 = Vector3Subtract(Vector3Add(center, Vector3Scale(right, halfSize)), Vector3Scale(up, halfSize));
        Vector3 p4 = Vector3Subtract(Vector3Subtract(center, Vector3Scale(right, halfSize)), Vector3Scale(up, halfSize));

        //Binds the decal texture
        rlSetTexture(d.texture->id);

        //Draws a textured quad with fading alpha
        rlBegin(RL_QUADS);
        rlColor4f(1.0f, 1.0f, 1.0f, 1.0f - (d.age / d.lifetime));//fade out over time (based on age %)

        rlTexCoord2f(0, 0); rlVertex3f(p1.x, p1.y, p1.z);
        rlTexCoord2f(1, 0); rlVertex3f(p2.x, p2.y, p2.z);
        rlTexCoord2f(1, 1); rlVertex3f(p3.x, p3.y, p3.z);
        rlTexCoord2f(0, 1); rlVertex3f(p4.x, p4.y, p4.z);

        rlEnd();
        rlSetTexture(0);
    }
}


//Estimates which face of a bounding box was hit and returns the surface normal
Vector3 EstimateNormalFromHit(Vector3 hit, BoundingBox box)
{
    const float threshold = 0.01f;
    //Check each axis for proximity to box face (min/max)
    if (fabs(hit.x - box.min.x) < threshold) return  { -1, 0, 0 };//Left face
    if (fabs(hit.x - box.max.x) < threshold) return  { 1, 0, 0 };//Right face
    if (fabs(hit.y - box.min.y) < threshold) return  { 0, -1, 0 };//Bottom face
    if (fabs(hit.y - box.max.y) < threshold) return  { 0, 1, 0 };//Top face
    if (fabs(hit.z - box.min.z) < threshold) return  { 0, 0, -1 };//Back face
    if (fabs(hit.z - box.max.z) < threshold) return  { 0, 0, 1 };//Front face

    //Fallback normal (assumes upward if hit is not close to any face)
    return  { 0, 1, 0 };
}