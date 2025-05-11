#include "EntitySpawner.h"
#include <raymath.h>

EntitySpawner::EntitySpawner(std::vector<Enemy>& enemiesRef)
    : enemies(enemiesRef)
{}


// Instead of creating an Enemy immediately, enqueue a portal effect:
void EntitySpawner::Spawn(int entityID, Vector3 position)
{
    SpawnEffect portal;
    portal.entityID = entityID;
    portal.pos = position;
    portal.timer = 0.0f;
    portal.scale = 0.0f;
    portal.waitTime = GetRandomValue(1000, 2000) / 1000.0f;//1-2 secs
    portal.state = State::Opening;
    effects.push_back(portal);
}

void EntitySpawner::init()
{
    portalTexture = LoadTexture("resources/Portal.png");
}

void EntitySpawner::Update(const Vector3& playerPos, std::vector<std::vector<std::vector<bool>>> navGrid)
{
    float dt = GetFrameTime();
    spawnTimer += dt;
    gameTime += dt;


    //Progression - every levelTick seconds, increase level
    if (level < maxLevel && gameTime >= (level + 1) * levelTick)
    {
        level++;
        maxEnemies += 2;
        spawnInterval = std::max(0.5f, spawnInterval * 0.8f);//20% faster spawns each level, limit to 0.5
    }


    //Gets the current amount of alive enemies in the game
    int aliveEnemyCount = 0;
    for (int index = 0; index < enemies.size(); index++)
    {
        if (enemies[index].IsAlive())
        {
            aliveEnemyCount++;
        }
    }

    //If the amount is less than our max allowed, allow spawning more
    if (aliveEnemyCount < maxEnemies)
    {
        //Attempt spawning on a suitable walkable tile
        if (spawnTimer >= spawnInterval)
        {
            spawnTimer = 0;

            int layers = (int)navGrid.size();
            if (layers > 0)//Must spawn higher than bottom layer
            {
                //Get grid cords
                int rows = (int)navGrid[0].size();
                int cols = (int)navGrid[0][0].size();

                //Try random positions until one fits criteria (BLACK tile (EMPTY SPACE) with a non black tile underneath (bounding box, wall/floor)
                for (int attempt = 0; attempt < 100; ++attempt)
                {
                    int layer = GetRandomValue(1, layers - 1);//Start at layer 1
                    int y = GetRandomValue(0, rows - 1);
                    int x = GetRandomValue(0, cols - 1);
                    if (!navGrid[layer][y][x]) continue;//Must be walkable

                    //Center of tile
                    Vector3 pos = { x + 0.5f, layer + 0.5f, y + 0.5f };

                    //Check distance from player
                    float dist = Vector3Distance(playerPos, pos);
                    if (dist < spawnRangeMin || dist > spawnRangeMax) continue;

                    //Found valid spawn, create a portal and bring in random enemy with difficulty setting
                    int enemyID = GetRandomEnemyID();
                    Spawn(enemyID, pos);
                    break;
                }
            }
        }
    }




    //Update all portals
    for (int i = (int)effects.size() - 1; i >= 0; --i)
    {
        auto& portal = effects[i];


        switch (portal.state) 
        {

            //Portal being created
        case State::Opening:

            portal.scale += growSpeed * dt;
            if (portal.scale >= maxScale) 
            {
                portal.scale = maxScale;
                portal.timer = 0;
                portal.state = State::Waiting;
            }
            break;



            //Waiting for demons to come out of the portal
        case State::Waiting:

            portal.timer += dt;
            if (portal.timer >= portal.waitTime)
            {
                enemies.emplace_back();
                Enemy& newEnemy = enemies.back();
                newEnemy.SetPosition(portal.pos);



                //Spawn enemies:
                switch (portal.entityID)
                {

                    //DEMON
                case 1:
                    newEnemy.health = 60;
                    newEnemy.size = 0.7f;
                    newEnemy.speed = 5.5f;
                    newEnemy.attackStrenght = 20;


                    newEnemy.enemyType = static_cast<Enemy::EnemyType>(static_cast<int>(newEnemy.enemyType) + 0);
                    break;



                    //IMP
                case 2:
                    newEnemy.health = 80;
                    newEnemy.size = 0.70f;
                    newEnemy.speed = 4.0f;
                    newEnemy.attackStrenght = 20;

                    newEnemy.enemyType = static_cast<Enemy::EnemyType>(static_cast<int>(newEnemy.enemyType) + 1);
                    break;


                    //SKELETON
                case 3:
                    newEnemy.health = 50;
                    newEnemy.speed = 3.5f;
                    newEnemy.attackFrameTime = 0.35f;
                    newEnemy.size = 0.7f;
                    newEnemy.attackStrenght = 20;

                    newEnemy.enemyType = static_cast<Enemy::EnemyType>(static_cast<int>(newEnemy.enemyType) + 2);
                    break;


                    //GUNNER
                case 4:
                    newEnemy.health = 120;
                    newEnemy.speed = 1;
                    newEnemy.size = 0.75f;
                    newEnemy.shotDamage = 18;

                    newEnemy.attackStrenght = 0;
                    newEnemy.canMelee = false;

                    newEnemy.enemyType = static_cast<Enemy::EnemyType>(static_cast<int>(newEnemy.enemyType) + 3);
                    break;


                    //BRUTE
                case 5:
                    newEnemy.health = 180;
                    newEnemy.speed = 2.5f;
                    newEnemy.attackFrameTime = 0.35f;
                    newEnemy.size = 0.85f;
                    newEnemy.stunable = true;
                    newEnemy.attackStrenght = 35;
                    
                    newEnemy.enemyType = static_cast<Enemy::EnemyType>(static_cast<int>(newEnemy.enemyType) + 4);
                    break;


                    //CHAMPION
                case 6:
                    newEnemy.health = 300;
                    newEnemy.speed = 1.25;
                    newEnemy.attackFrameTime = 0.35f;
                    newEnemy.size = 0.95f;
                    newEnemy.stunable = false;
                    newEnemy.attackStrenght = 60;
                    newEnemy.shotDamage = 14;
                    newEnemy.shotsToFireMin = 1;
                    newEnemy.shotsToFireMax = 1;

                    newEnemy.enemyType = static_cast<Enemy::EnemyType>(static_cast<int>(newEnemy.enemyType) + 5);
                    break;

                }





                //Spawn at position pos
            newEnemy.SetPosition(portal.pos);

            //Close the portal
            portal.state = State::Closing;
            portal.scale = maxScale;

            }
            break;


            //Clsoing portal
        case State::Closing:

            portal.scale -= shrinkSpeed * dt;
            if (portal.scale <= 0) 
            {
                effects.erase(effects.begin() + i);
                continue;
            }

            break;
        }
    }
}



void EntitySpawner::Draw(const Camera3D& camera)
{
    for (auto& portal : effects) DrawBillboard(camera, portalTexture, portal.pos, portal.scale * (float)portalTexture.width, WHITE);
}

int EntitySpawner::GetRandomEnemyID() const
{

    //As level increases, we boost weights of higher ids (easier enemies more likely to spawn early on)
    std::vector<float> weights(6);


    //Decay decreases over time, higher id enemies become more probable
    for (int i = 0; i < 6; i++)
    {
        float decay = 1.0f - (float)level / (float)maxLevel * 0.5f;
        decay = std::max(decay, 0.3f);//Stop it going negative
        weights[i] = powf(1.0f / float(i + 1), decay);//weight is (1/(i+1))^decay    (i=0 always 1^decay = 1 == easiest)     (i=5 is (1/6)^decay == small number for hard enemies)
    }

    //Normalize to get valid probabilities
    float sum = std::accumulate(weights.begin(), weights.end(), 0.0f);
    for (auto& weight : weights) weight /= sum;

    //Pick enemy by going through our weights, adding htem up until we reach or pass random
    float random = GetRandomValue(0, 10000) / 10000.0f;//picks a random number between 0 and 1
    float accumulateWeight = 0.0f;//The index we stop at will give us our id
    for (int i = 0; i < 6; i++)
    {
        accumulateWeight += weights[i];
        if (random <= accumulateWeight) return i + 1;//Return id in range 1 - 6
    }
    return 6;
}


//Removes all enemies that are not alive
void EntitySpawner::DespawnDeadEnemies()
{
    enemies.erase( std::remove_if(enemies.begin(), enemies.end(), [](const Enemy& enemy) { return !enemy.IsAlive(); }), enemies.end());
}