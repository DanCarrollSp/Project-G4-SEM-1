// Enemy.cpp
#include "Enemy.h"


//Animation and texture sets
bool Enemy::textureSetLoaded = false;
std::array<Enemy::AnimationSet, static_cast<size_t>(Enemy::EnemyType::Count)>Enemy::animationSets;


//Constructor (defaults if spawner hasnt given any)
Enemy::Enemy()
    : enemyType(EnemyType::Demon),
    speed(4.5f),
    position({ 5.0f, 0.5f, 5.0f }),
    lastPosition(position),
    texturesLoaded(false),
    frameIndex(0),
    lastAnimationTime(0.0),
    health(100),
    hit(false),
    hitTime(0.0),
    hitDuration(0.25),
    isAlive(true),
    onDeath(false),
    currentPathIndex(0),
    lastTargetPosition({ FLT_MAX, FLT_MAX, FLT_MAX })
{
    position.y = size * 0.5f + 0.1f + 1;
    updateHitbox();
}

Enemy::~Enemy() {}



//Forcibly set position and update hitbox pos
void Enemy::SetPosition(const Vector3& newPos)
{
    //Make sure hitbox is up to date
    updateHitbox();

    //Set x/z, drop y to ground level based on size
    position.x = newPos.x;
    position.z = newPos.z;

    float tileFloorY = newPos.y - 0.5f;
    float bottomHover = 0.1f;
    position.y = tileFloorY + (size * 0.5f) + bottomHover;


    //Record which layer we’re on
    layer = static_cast<int>(std::floor(newPos.y));

    //Update new hitbox
    updateHitbox();
}

void Enemy::Update() 
{
    //Debug movement
    if (IsKeyDown(KEY_RIGHT)) position.x += speed * GetFrameTime();
    if (IsKeyDown(KEY_LEFT))  position.x -= speed * GetFrameTime();
    if (IsKeyDown(KEY_UP))    position.z -= speed * GetFrameTime();
    if (IsKeyDown(KEY_DOWN))  position.z += speed * GetFrameTime();


    //Update enemy animations every frame
    animate();


    //Clear enemy for deletion in the unlikely event they become stuck
    if (hitbox.max.y <= 1.2f)
    {
        health = 100;
        SetPosition({ -10, -10, -10 });
    }
}

void Enemy::Draw(Camera camera)
{
    //Draw enemy
    DrawBillboard(camera, currentTexture, position, size, WHITE);
    //DrawBoundingBox(GetBoundingBox(), RED);//Debug
}



//Return if player raycast gun is hitting or not
Vector3 Enemy::collision(Ray ray)
{
    RayCollision col = GetRayCollisionBox(ray, hitbox);
    return col.hit ? col.point : Vector3Zero();
}



void Enemy::vectorCollision(const std::vector<BoundingBox>& walls, const std::vector<Enemy*>& others)
{
    //If were dead dont bother with collisions
    if (!isAlive) return;


    //Update hitbox pos
    updateHitbox();


    //Check if we can step up onto the colliding box
    bool stepped = false;
    for (auto& wall : walls) 
    {
        if (!CheckCollisionBoxes(hitbox, wall)) continue;

        float footY = hitbox.min.y, topY = wall.max.y;
        float destinationY = topY - footY;

        if (!stepped && std::fabs(destinationY) <= maxStepHeight)
        {
            position.y += destinationY;
            updateHitbox();
            stepped = true;
            continue;
        }


        //Checks which direction in X we are overlapping, shallowest penetration pushes back out
        float overLapLeftX = wall.max.x - hitbox.min.x, overLapRightX = hitbox.max.x - wall.min.x;
        float shallowestX = (std::fabs(overLapLeftX) < std::fabs(overLapRightX)) ? overLapLeftX : -overLapRightX;

        float overLapLeftZ = wall.max.z - hitbox.min.z, overLapRightZ = hitbox.max.z - wall.min.z;
        float shallowestZ = (std::fabs(overLapLeftZ) < std::fabs(overLapRightZ)) ? overLapLeftZ : -overLapRightZ;

        if (std::fabs(shallowestX) < std::fabs(shallowestZ)) position.x += shallowestX;
        else position.z += shallowestZ;

        //Update new positions hitbox
        updateHitbox();
    }


    for (auto* otherEnemy : others)
    {
        //Only collide with allive enemies
        if (otherEnemy == this || !otherEnemy->IsAlive()) continue;
        BoundingBox box = otherEnemy->GetBoundingBox();
        //Check if colliding at all
        if (!CheckCollisionBoxes(hitbox, box)) continue;

        //Get horizontal push direction
        Vector3 dir = Vector3Subtract(position, otherEnemy->position);
        if (Vector3Length(dir) == 0.0f) dir = { 0.01f, 0.0f, 0.0f };
        dir = Vector3Normalize(dir);


        //How strong the push shld be
        constexpr float pushAmt = 0.02f;
        Vector3 offset = Vector3Scale(dir, pushAmt);


        //Push THIS enemy
        position.x += offset.x;
        position.z += offset.z;
        updateHitbox();
        //Push the other enemy
        otherEnemy->position.x -= offset.x;
        otherEnemy->position.z -= offset.z;
        otherEnemy->updateHitbox();
    }

}


void Enemy::updateHitbox()
{
    float baseHalfW = 0.23f, baseHalfH = 0.49f;
    float halfW = baseHalfW * size;
    float halfH = baseHalfH * size;

    hitbox.min = { position.x - halfW, position.y - halfH, position.z - halfW };
    hitbox.max = { position.x + halfW, position.y + halfH, position.z + halfW };
}


//Movement and pathfinding
void Enemy::Move(const Vector3& target, const std::vector<std::vector<std::vector<bool>>>& navGrid,
    const std::vector<BoundingBox>& stairs, const std::vector<BoundingBox>& walls,
    const std::vector<Enemy*>& allEnemies, float deltaTime, Player& player)
{
    //Gravity and hitbox updates, including stepping up
    ApplyPhysics(deltaTime, walls);
    float dt = GetTime();


    //Try shooting, if were mid shoot no need to change pos so return
    HandleShooting(target, walls, player, deltaTime);
    if (isShooting) return;
    //Same for melee
    if (canMelee) HandleAttack(target, player);
    if (isAttacking) return;


    //If we need a new path (no path already or player moved or time elapsed to look for new path)
    bool needRepath = (Vector3Distance(target, lastTargetPosition) > 0.1f) || currentPathIndex >= (int)path.size() || (dt - lastPathTime) >= 1.0;
    if (needRepath && !pathPending)
    {
        //Get the result of the A* path before we need it
        pathFuture = std::async(std::launch::async, &Enemy::ComputePath, position, target, navGrid, stairs);
        pathPending = true;
    }

    //When our future path has been found
    if (pathPending && pathFuture.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready)
    {
        //Get path
        auto newPath = pathFuture.get();
        pathPending = false;//Tell our pathfinder we now are using their future path and will need a new one for next calc

        //Only accept it if it gave us more than just a start node
        if (newPath.size() > 1)
        {
            //Move our new found path into our actual path
            path = std::move(newPath);
            currentPathIndex = 1;

            //Update where were going to where weve last attempted to go and when we tried to
            lastTargetPosition = target;
            lastPathTime = dt;
        }
    }


    //Fallback to just straight chase the enemy if no path found
    if (path.size() <= 1)
    {
        //Get the direction and distance to target
        Vector3 directToTarget = Vector3Subtract(target, position); directToTarget.y = 0;
        float dist = Vector3Length(directToTarget);

        //Move to directly to target
        if (dist > 0.01f)
        {
            Vector3 dir = Vector3Scale(directToTarget, 1.0f / dist);
            position.x += dir.x * speed * deltaTime;
            position.z += dir.z * speed * deltaTime;
            updateHitbox();
        }
        
        //Still do physic and collisions
        ApplyPhysics(deltaTime, walls);
        vectorCollision(walls, allEnemies);
        return;
    }



    //If we do have a proper path follow it
    //If the part of the path were on isnt the end
    if (currentPathIndex < path.size())
    {
        //Move from our current path index to the next
        Vector3 next = path[currentPathIndex];//Get waypoint
        Vector3 dir = { next.x - position.x, 0, next.z - position.z };//Horizontal vector from our ps to the waypoint


        //How far is the weighpoint
        float dist = Vector3Length(dir);
        if (dist > 0.01f)//Only move if were not basically there already
        {
            //Normalize
            dir = Vector3Scale(dir, 1.0f / dist);

            //Move in that direction by speed
            float movement = speed * deltaTime;
            position.x += dir.x * movement;
            position.z += dir.z * movement;
            updateHitbox();


            //If weve arrived close enough to where we were going snap to it (no jitter)
            if (Vector3Distance({ position.x,0,position.z }, { next.x,    0,next.z }) < 0.05f)
            {
                position.x = next.x;
                position.z = next.z;
                updateHitbox();

                //Advance to next waypoint on the path next update
                currentPathIndex++;
            }
        }
    }


    //Pysics and colisions after movements (push us back or up if neccisary before next allowed movement
    vectorCollision(walls, allEnemies);
    ApplyPhysics(deltaTime, walls);
}



void Enemy::ApplyPhysics(float deltaTime, const std::vector<BoundingBox>& walls)
{
    //Gravity vars
    float gravity = -25.0f;
    float maxGravity = -50.0f;
    float step = -0.3f;
    float hoverOffset = 0.01f;


    //Set Y velocity to gravity speed and cap to max speed
    velocityY += gravity * deltaTime;
    if (velocityY < maxGravity) velocityY = maxGravity;


    //Calc how much we want to move on Y this frame
    float remain = velocityY * deltaTime;
    std::vector<BoundingBox> ground = walls;
    float halfH = (hitbox.max.y - hitbox.min.y) * 0.49f;

    //Step down incrementally until all verticle movement remaining to do this frame is done (lets us stop at anypoint if weve landed)
    while (remain < 0.0f)
    {
        float stepAmount = std::max(remain, step);
        Vector3 prevPos = position;

        //Step down and update hitbox
        position.y += stepAmount;
        updateHitbox();


        //Resolve if weve hit the floor or not
        bool resolved = false;
        float feetY = prevPos.y - halfH;

        float bestLow = -FLT_MAX, bestSame = -FLT_MAX;
        bool  foundLow = false, foundSame = false, sideHit = false;

        for (auto& box : ground)
        {
            //Skip out over checks if we havent hit the floor
            if (!CheckCollisionBoxes(hitbox, box)) continue;


            //If were above the box
            if (feetY >= box.max.y - hoverOffset)
            {
                //If the drop below is big
                if (box.max.y < (feetY - hoverOffset - 0.001f))
                {
                    //Get the highest surface below
                    if (box.max.y > bestLow)
                    {
                        bestLow = box.max.y;
                        foundLow = true;
                    }

                }
                else//(Drop is tiny)
                {
                    //Get the highest part of the same nearly level surface
                    if (box.max.y > bestSame)
                    {
                        bestSame = box.max.y;
                        foundSame = true;
                    }

                }
            }
            else sideHit = true;//Were not above the box so its a side/horizontal hit
        }



        //Reolve gravity to floor collisions
        if (foundLow)
        {
            position.y = bestLow + halfH + hoverOffset;
            updateHitbox();
            velocityY = 0;
            resolved = true;
        }
        else if (foundSame)
        {
            position.y = bestSame + halfH + hoverOffset;
            updateHitbox();
            velocityY = 0;
            resolved = true;
        }
        
        //Resolve side hits
        if (sideHit)
        {
            position.y = prevPos.y;
            updateHitbox();
            velocityY = 0;
            resolved = true;
        }


        //Stop stepping checks on hitboxes if done or reduce remaining steps to check if not
        if (resolved) remain = 0;
        else remain -= stepAmount;
    }
}


std::vector<Vector3> Enemy::ComputePath(const Vector3& start, const Vector3& target, const std::vector<std::vector<std::vector<bool>>>& navGrid, const std::vector<BoundingBox>& stairs)
{
    //Stair paths to next or last layer
    std::vector<StairEdge> edges;

    for (auto& stair : stairs)
    {
        int xPos = (int)std::floor(stair.min.x);
        int zPos = (int)std::floor(stair.min.z);
        int layer1 = (int)std::floor(stair.min.y);
        int layer2 = (int)std::floor(stair.max.y);

        //Edges of the grid path to different layers
        edges.push_back({ xPos,zPos,layer1, xPos,zPos,layer2 });
        edges.push_back({ xPos,zPos,layer2, xPos,zPos,layer1 });
    }


    //Raw manhatten A* grid path
    auto rawPath = AStarPath((Vector3&)start, (Vector3&)target, navGrid, edges);
    std::vector<Vector3> finalPath;

    int index = 0;
    int wayPoints = (int)rawPath.size();

    //Loop over all waypoints in raw path to smooth the ones on our layer
    while (index < wayPoints)
    {
        //Get current layer
        int layer = (int)std::floor(rawPath[index].y);

        //Start 1 past our current path index on that layer (keeps us moving forward)
        int currentLayersIndex = index + 1;
        //Keep advancing as long as there are remaining points on the same layer
        while (currentLayersIndex < wayPoints && (int)std::floor(rawPath[currentLayersIndex].y) == layer) ++currentLayersIndex;
        //Smooth the raw path on the current layer
        auto smoothedPoints = SmoothPath( { rawPath.begin() + index, rawPath.begin() + currentLayersIndex }, navGrid[layer] );

        //Insert our smoothed segemented path into a single final path
        if (finalPath.empty()) finalPath = smoothedPoints;
        else finalPath.insert(finalPath.end(), smoothedPoints.begin() + 1, smoothedPoints.end());
        index = currentLayersIndex;
    }
    return finalPath;
}


void Enemy::RecalculatePath(const Vector3& target, const std::vector<std::vector<std::vector<bool>>>& navGrid, const std::vector<BoundingBox>& stairs)
{
    path = ComputePath(position, target, navGrid, stairs);
    currentPathIndex = 1;
}




void Enemy::HandleAttack(const Vector3& target, Player& player)
{
    float dt = GetTime();
    const auto& anim = animationSets[static_cast<size_t>(enemyType)];

    //If mid attack
    if (isAttacking && canMelee)
    {
        int attackFramesIndex = int((dt - attackTimer) / attackFrameTime);
        if (attackFramesIndex < (int)anim.attack.size()) {
            currentTexture = anim.attack[attackFramesIndex];
            return;
        }

        //Attack anim just ended, do damage if the player still in range
        Vector3 delta = Vector3Subtract(player.position, position);
        float deltaY = fabs(delta.y);
        delta.y = 0;

        //Deal damage if still too close to enemy when their attack finishes
        if (deltaY <= 1.25f && Vector3Length(delta) <= attackRange)
        {
            player.takeDamage(attackStrenght);
        }

        //Reset attacking
        isAttacking = false;
        lastAnimationTime = dt;
        frameIndex = 0;
        currentTexture = anim.walk[0];
        return;
    }


    //Try start an attack
    Vector3 horizontalDistance = Vector3Subtract(target, position);
    horizontalDistance.y = 0;

    //Set melee attacking to true if player too close
    if (Vector3Length(horizontalDistance) <= attackRange)
    {

        hasMeleeAttacked = true;
        isAttacking = true;
        attackTimer = dt;

        isAttacking = true;
        attackTimer = dt;
        frameIndex = 0;
        if (anim.attack.size() > 0)currentTexture = anim.attack[0];
    }

}


//Damge this enemy
void Enemy::TakeDamage(int amount)
{
    health -= amount;

    //If the attack killed the enemy set to dead
    if (health <= 0)
    {
        health = 0;
        isAlive = false;

    }
    //If still alive after the attack, if the attack did sizable damage stun them
    else if (amount > 20 && stunable == true)
    {
        hit = true;
        hitTime = GetTime();
        speed = 0;
    }
}




void Enemy::animate()
{

    //Load all texture sets once
    if (!textureSetLoaded)
    {


        //DEMON
        {
            auto& set = animationSets[static_cast<size_t>(EnemyType::Demon)];
            set.walk =
            {
                LoadTexture("resources/enemies/Demon/D1.png"),
                LoadTexture("resources/enemies/Demon/D2.png"),
                LoadTexture("resources/enemies/Demon/D3.png"),
                LoadTexture("resources/enemies/Demon/D4.png"),
            };

            set.stunned = LoadTexture("resources/enemies/Demon/D5.png");

            set.attack =
            {
                LoadTexture("resources/enemies/Demon/DDD1.png"),
                LoadTexture("resources/enemies/Demon/DDD2.png"),
                LoadTexture("resources/enemies/Demon/DDD3.png"),
            };

            set.death =
            {
                LoadTexture("resources/enemies/Demon/DD1.png"),
                LoadTexture("resources/enemies/Demon/DD2.png"),
                LoadTexture("resources/enemies/Demon/DD3.png"),
                LoadTexture("resources/enemies/Demon/DD4.png"),
                LoadTexture("resources/enemies/Demon/DD5.png"),
                LoadTexture("resources/enemies/Demon/DD6.png"),
            };
        }


        //IMP
        {
            auto& set = animationSets[static_cast<size_t>(EnemyType::Imp)];
            set.walk =
            {
                LoadTexture("resources/enemies/Imp/walk1.png"),
                LoadTexture("resources/enemies/Imp/walk2.png"),
                LoadTexture("resources/enemies/Imp/walk3.png"),
                LoadTexture("resources/enemies/Imp/walk4.png"),
            };

            set.stunned = LoadTexture("resources/enemies/Imp/stunned.png");

            set.attack =
            {
                LoadTexture("resources/enemies/Imp/attack1.png"),
                LoadTexture("resources/enemies/Imp/attack2.png"),
                LoadTexture("resources/enemies/Imp/attack3.png"),
            };

            set.death =
            {
                LoadTexture("resources/enemies/Imp/dead1.png"),
                LoadTexture("resources/enemies/Imp/dead2.png"),
                LoadTexture("resources/enemies/Imp/dead3.png"),
                LoadTexture("resources/enemies/Imp/dead4.png"),
                LoadTexture("resources/enemies/Imp/dead5.png"),
            };


            //SKELETON
            {
                auto& set = animationSets[static_cast<size_t>(EnemyType::Skeleton)];
                set.walk =
                {
                    LoadTexture("resources/enemies/Skeleton/walk1.png"),
                    LoadTexture("resources/enemies/Skeleton/walk2.png"),
                    LoadTexture("resources/enemies/Skeleton/walk3.png"),
                    LoadTexture("resources/enemies/Skeleton/walk4.png"),
                };

                set.stunned = LoadTexture("resources/enemies/Skeleton/stunned.png");

                set.attack =
                {
                    LoadTexture("resources/enemies/Skeleton/attack1.png"),
                    LoadTexture("resources/enemies/Skeleton/attack2.png")
                };

                set.death =
                {
                    LoadTexture("resources/enemies/Skeleton/dead1.png"),
                    LoadTexture("resources/enemies/Skeleton/dead2.png"),
                    LoadTexture("resources/enemies/Skeleton/dead3.png"),
                    LoadTexture("resources/enemies/Skeleton/dead4.png"),
                    LoadTexture("resources/enemies/Skeleton/dead5.png"),
                };



                //GUNNER
                {
                    auto& set = animationSets[static_cast<size_t>(EnemyType::Gunner)];
                    set.walk =
                    {
                        LoadTexture("resources/enemies/Gunner/walk1.png"),
                        LoadTexture("resources/enemies/Gunner/walk2.png"),
                        LoadTexture("resources/enemies/Gunner/walk3.png"),
                        LoadTexture("resources/enemies/Gunner/walk4.png"),
                    };

                    set.stunned = LoadTexture("resources/enemies/Gunner/stunned.png");

                    set.shoot =
                    {
                        LoadTexture("resources/enemies/Gunner/attack1.png"),
                        LoadTexture("resources/enemies/Gunner/attack2.png")
                    };

                    set.death =
                    {
                        LoadTexture("resources/enemies/Gunner/dead1.png"),
                        LoadTexture("resources/enemies/Gunner/dead2.png"),
                        LoadTexture("resources/enemies/Gunner/dead3.png"),
                        LoadTexture("resources/enemies/Gunner/dead4.png"),
                        LoadTexture("resources/enemies/Gunner/dead5.png"),
                        LoadTexture("resources/enemies/Gunner/dead6.png"),
                        LoadTexture("resources/enemies/Gunner/dead7.png"),
                        LoadTexture("resources/enemies/Gunner/dead8.png"),
                        LoadTexture("resources/enemies/Gunner/dead9.png"),
                        LoadTexture("resources/enemies/Gunner/dead10.png"),
                    };



                    //BRUTE
                    {
                        auto& set = animationSets[static_cast<size_t>(EnemyType::Brute)];
                        set.walk =
                        {
                            LoadTexture("resources/enemies/Brute/walk1.png"),
                            LoadTexture("resources/enemies/Brute/walk2.png"),
                            LoadTexture("resources/enemies/Brute/walk3.png"),
                            LoadTexture("resources/enemies/Brute/walk4.png"),
                        };

                        set.stunned = LoadTexture("resources/enemies/Brute/stunned.png");

                        set.attack =
                        {
                            LoadTexture("resources/enemies/Brute/attack1.png"),
                            LoadTexture("resources/enemies/Brute/attack2.png"),
                            LoadTexture("resources/enemies/Brute/attack3.png"),
                        };

                        set.death = {
                            LoadTexture("resources/enemies/Brute/dead1.png"),
                            LoadTexture("resources/enemies/Brute/dead2.png"),
                            LoadTexture("resources/enemies/Brute/dead3.png"),
                            LoadTexture("resources/enemies/Brute/dead4.png"),
                            LoadTexture("resources/enemies/Brute/dead5.png"),
                            LoadTexture("resources/enemies/Brute/dead6.png"),
                            LoadTexture("resources/enemies/Brute/dead7.png"),
                            LoadTexture("resources/enemies/Brute/dead8.png"),
                            LoadTexture("resources/enemies/Brute/dead9.png"),
                            LoadTexture("resources/enemies/Brute/dead10.png"),
                        };



                        //CHAMPION
                        {
                            auto& set = animationSets [static_cast<size_t>(EnemyType::Champion)];
                            set.walk =
                            {
                                LoadTexture("resources/enemies/Champion/walk1.png"),
                                LoadTexture("resources/enemies/Champion/walk2.png"),
                                LoadTexture("resources/enemies/Champion/walk3.png"),
                                LoadTexture("resources/enemies/Champion/walk4.png"),
                            };
                            set.attack =
                            {
                                LoadTexture("resources/enemies/Champion/attack2.png"),
                                LoadTexture("resources/enemies/Champion/attack1.png")
                            };

                            set.shoot =
                            {
                                LoadTexture("resources/enemies/Champion/shoot1.png"),
                                LoadTexture("resources/enemies/Champion/shoot2.png"),
                            };

                            set.death =
                            {
                                LoadTexture("resources/enemies/Champion/dead1.png"),
                                LoadTexture("resources/enemies/Champion/dead2.png"),
                                LoadTexture("resources/enemies/Champion/dead3.png"),
                                LoadTexture("resources/enemies/Champion/dead4.png"),
                                //LoadTexture("resources/enemies/Champion/dead5.png"),//Commented out because it looks better without this frame
                                LoadTexture("resources/enemies/Champion/dead6.png"),
                                LoadTexture("resources/enemies/Champion/dead7.png"),

                            };
                        }

                        //Set textures to loaded
                        textureSetLoaded = true;
                    }
                }
            }
        }
    }


    //On first animate call pick our set based on enemy type
    if (!texturesLoaded)
    {
        const auto& animSet = animationSets[static_cast<size_t>(enemyType)];
        currentTexture = animSet.walk[0];
        texturesLoaded = true;
    }

    double dt = GetTime();
    const auto& animSet = animationSets[static_cast<size_t>(enemyType)];


    //Death
    if (!isAlive)
    {
        //If its the start of the dead anim
        if (!onDeath)
        {
            frameIndex = 0;
            onDeath = true;
        }

        //Move
        if (dt - lastAnimationTime >= animFramTime)
        {
            lastAnimationTime = dt;
            if (frameIndex < (int)animSet.death.size() - 1) frameIndex++;
            currentTexture = animSet.death[frameIndex];
        }
        return;
    }


    if (isShooting)
    {
        //Leaves currentTexture as HandleShooting last set it
        return;
    }

    //Stunned
    if (stunable == true)
    {
        if (hit)
        {
            currentTexture = animSet.stunned;

            if (dt - hitTime >= hitDuration)
            {
                hit = false;
                speed = 3.0f;
            }
            lastPosition = position;
            return;
        }
    }


    //Walk
    bool isMoving = Vector3Distance(position, lastPosition) > 0.001f;
    if (isMoving)
    {
        if (dt - lastAnimationTime >= 0.125)
        {
            lastAnimationTime = dt;
            frameIndex = (frameIndex + 1) % animSet.walk.size();
            currentTexture = animSet.walk[frameIndex];
        }
    }
    else
    {
        frameIndex = 0;
        currentTexture = animSet.walk[0];
    }
    lastPosition = position;
}



void Enemy::HandleShooting(const Vector3& target, const std::vector<BoundingBox>& walls, Player& player, float depreciated)
{
    //Get anim set based on enemy type
    auto& animSet = animationSets[size_t(enemyType)];
    auto& shootAnims = animSet.shoot;
    if (shootAnims.empty()) return;


    //Check for line of sight
    Vector3 toPlayer = Vector3Subtract(player.position, position);
    float dist = Vector3Length(toPlayer);
    Ray ray = { position, Vector3Normalize(toPlayer) };
    bool los = true;//Default to true


    //If walls collide with the ray to the player set los to false
    for (auto& wall : walls)
    {
        auto rc = GetRayCollisionBox(ray, wall);
        if (rc.hit && Vector3Distance(ray.position, rc.point) < dist)
        {
            los = false;
            break;
        }
    }


    float dt = GetTime();
    if (los)
    {
        if (!hasLOS)
        {
            hasLOS = true;
            lastSightTime = dt;
        }
        else if (!isShooting && !hasMeleeAttacked && (dt - lastSightTime) >= 2.0)
        {
            //Start the volley of shots based on random range of how many can be shot per aim set in spawner
            isShooting = true;
            hasMeleeAttacked = true;
            shotsToFire = GetRandomValue(shotsToFireMin, shotsToFireMax);
            shotsFired = 0;
            shootPhase = ShootState::Aiming;
            shootStateTimer = dt;
            currentTexture = shootAnims[0];//Aiming is always first frame of shooting
        }
    }
    else
    {
        //Reset los and inturn timers after done shooting
        hasLOS = false;
    }


    //Abort shooting mid volley if los lost
    if (isShooting && !los)
    {
        isShooting = false;
        hasMeleeAttacked = false;
        hasLOS = false;
        lastSightTime = dt;

        //revert to walk frame
        currentTexture = animationSets[size_t(enemyType)].walk[0];
        return;
    }


    //Past this is logic for firing
    if (!isShooting) return;

    //Swap from aiming to shooting and swap rapidly back and forth if unleashing a volley of shots
    if (shootPhase == ShootState::Aiming)
    {
        //First shot takes 2 secs of aiming then 0.75 per shot in the vollet after
        double waitDur = (shotsFired == 0 ? 2.0 : 0.075);

        if (dt - shootStateTimer >= waitDur)
        {
            //Fire
            shootPhase = ShootState::Firing;
            shootStateTimer = dt;
            currentTexture = shootAnims[1];//Show second frame in shooting - gun muzzle flash of enemy weapon
            player.takeDamage(shotDamage);
        }
    }
    else//If were in firing state
    {
        if (dt - shootStateTimer >= 0.1)
        {
            shotsFired++;

            if (shotsFired >= shotsToFire)
            {
                //Done shooting reset
                isShooting = false;
                hasMeleeAttacked = false;
                hasLOS = false;
                lastSightTime = dt;
                lastAnimationTime = dt;
                frameIndex = 0;
                currentTexture = animationSets[size_t(enemyType)].walk[0];
            }
            else
            {
                //More shots to go, revert instantly to aiming for next shot
                shootPhase = ShootState::Aiming;
                shootStateTimer = dt;
                currentTexture = shootAnims[0];//Reset back to shot1 aiming anim
            }
        }
    }
}