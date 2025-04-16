#include "EntitySpawner.h"

EntitySpawner::EntitySpawner(std::vector<Enemy>& enemiesRef)
    : enemies(enemiesRef)
{}


//Spawns a new enemy of our choice at the given position
void EntitySpawner::Spawn(int entityID, Vector3 position)
{
    Enemy newEnemy;
    newEnemy.SetPosition(position);

    //Different enemy types:
    switch (entityID)
    {
    case 0:
        //default
        break;
    case 1:
        //e.g newEnemy.SetSpeed(5.0f);
        //e.g newEnemy.SetHealth(200);
        break;
        //other varients
    }

    enemies.push_back(newEnemy);
}


//Removes all enemies that are not alive
void EntitySpawner::DespawnDeadEnemies()
{
    enemies.erase( std::remove_if(enemies.begin(), enemies.end(), [](const Enemy& enemy) { return !enemy.IsAlive(); }), enemies.end());
}