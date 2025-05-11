#ifndef ENTITY_SPAWNER_H
#define ENTITY_SPAWNER_H

#include "raylib.h"
#include <vector>
#include <numeric>

#include "Enemy.h"


class EntitySpawner
{
public:

    //Reference to the enemies vector so Spawn can add new enemies
    EntitySpawner(std::vector<Enemy>& enemiesRef);

    //Spawns a new enemy at the given position
    void Spawn(int entityID, Vector3 position);
    void DespawnDeadEnemies();
    float spawnInterval = 3.0f;
    int maxEnemies = 3;

    //
    void init();
    void Update(const Vector3& playerPos, std::vector<std::vector<std::vector<bool>>> navGrid);
    void Draw(const Camera3D& camera);

private:

    std::vector<Enemy>& enemies;//Vectir of our enemies
    //std::vector<std::vector<std::vector<bool>>> navGrid;//nav grid for spawning

    //Portal
    enum class State { Opening, Waiting, Closing };
    struct SpawnEffect
    {
        int entityID;//Enemy type
        Vector3 pos;
        float timer;//Waiting open portal for enemy to spawn
        float scale;//Current portal scale
        float waitTime;//2-3 secs of portal being open before enemy appears
        State state;//State of the portal
    };


    Texture2D portalTexture;
    std::vector<SpawnEffect> effects;


    //Portal attributes
    float growSpeed = 0.005f;
    float maxScale = 0.01f;//Full portal size
    float closeGrowSpeed = 2.0f;
    float shrinkSpeed = 0.025f;



    //Spawn timer
    float spawnTimer = 0.0f;
    //Random spawn range (distance from player a portal can appear at
    float spawnRangeMin = 5.0f;
    float spawnRangeMax = 20.0f;



    //Spawn logic
    int GetRandomEnemyID() const;
    //Progression
    float gameTime = 0.0f;//Total game elapsed time
    float levelTick = 30.0f;//Every 30s level up
    int level = 0;//Current progression level
    int maxLevel = 10;//Cap progression

};

#endif