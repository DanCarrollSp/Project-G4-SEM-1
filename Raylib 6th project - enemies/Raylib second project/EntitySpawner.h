#ifndef ENTITY_SPAWNER_H
#define ENTITY_SPAWNER_H

#include "raylib.h"
#include <vector>
#include "Enemy.h"

class EntitySpawner
{
public:

    //Reference to the enemies vector so Spawn can add new enemies
    EntitySpawner(std::vector<Enemy>& enemiesRef);

	//Spawns a new enemy at the given position
    void Spawn(int entityID, Vector3 position);
    void DespawnDeadEnemies();



private:

    std::vector<Enemy>& enemies;//Vectir of our enemies
};

#endif // ENTITY_SPAWNER_H
