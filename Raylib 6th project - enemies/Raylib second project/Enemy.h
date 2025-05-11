// Enemy.h
#ifndef ENEMY_H
#define ENEMY_H


#include "raylib.h"
#include "raymath.h"

#include <cmath>
#include <cfloat>
#include <vector>

#include <future>
#include <chrono>

#include <array>

#include <iostream>

#include "Pathfinding.h"
#include "Player.h"//To deal damage


class Enemy
{

public:

    //Constructor and destructor
    Enemy();
    ~Enemy();

    //
    Enemy(const Enemy&) = delete;
    Enemy& operator=(const Enemy&) = delete;
    Enemy(Enemy&&) = default;
    Enemy& operator=(Enemy&&) = default;


    //Set position after construction
    void SetPosition(const Vector3& newPos);


    //Main functions
    void Update();
    void Draw(Camera camera);



    //Collision detection
    Vector3 collision(Ray ray);//Returns if we (the player) have hit detection on the enemy hitbox
    void vectorCollision(const std::vector<BoundingBox>& walls, const std::vector<Enemy*>& otherEnemies);//Stops us walking through walls, enemies and falling through floors and stairs
    BoundingBox GetBoundingBox() const { return hitbox; }
    BoundingBox hitbox;


    //Movement and pathfinding
    void Move(const Vector3& target, const std::vector<std::vector<std::vector<bool>>>& navGrid,
        const std::vector<BoundingBox>& stairs, const std::vector<BoundingBox>& walls,
        const std::vector<Enemy*>& allEnemies, float deltaTime, Player& player);

    //Path updating
    double lastPathTime = 0.0;
    void RecalculatePath(const Vector3& target, const std::vector<std::vector<std::vector<bool>>>& navGrid, const std::vector<BoundingBox>& stairs);

    //Pathfinding data
    std::vector<Vector3> path;
    int currentPathIndex;
    Vector3 lastTargetPosition;



    //Update enemies hitbox to its position
    void updateHitbox();

    
    //Health and alive
    void TakeDamage(int amount);
    bool IsAlive() const { return isAlive; }
    bool onDeath;



    //Atributes
    Vector3 position;
    Vector3 lastPosition;
    int layer;
    

    int health;
    bool isAlive;
    float size = 1;
    float speed;

    bool stunable = true;
    float animFramTime = 0.125;
    bool hit;
    double hitTime;
    double hitDuration;
    
    
 
    //Where we load and cycle character imgs
    void animate();
    //Animation
    enum class EnemyType { Demon = 0, Imp, Skeleton, Gunner, Brute, Champion, Count};
    
    //Holds all the frames for enemy animations
    struct AnimationSet
    {
        std::vector<Texture2D> walk;
        Texture2D stunned;
        std::vector<Texture2D> attack;
        std::vector<Texture2D> shoot;
        std::vector<Texture2D> death;
    };

    EnemyType enemyType;
    Texture2D currentTexture;

    //Load and store animations
    static bool textureSetLoaded;
    static std::array<AnimationSet, int(EnemyType::Count)> animationSets;

    bool texturesLoaded;
    int frameIndex;
    float lastAnimationTime;




    //Physics
    void ApplyPhysics(float deltaTime, const std::vector<BoundingBox>& walls);//Applies gravity
    //bool TryStepUp(float deltaY, const std::vector<BoundingBox>& obstacles);//Tests if the enemy can climb a low world bounding box, if can, step up onto it, if cant, collide as expected with the wall (Now in vector collision)
    float maxStepHeight = 0.9;//Max allowed step hight (from min of bounding box)
    float GroundHover = 0.05f;//How much we make the enemy hover off the ground (avoids colliding with floor and stopping movement, instead handled by physics)
    float velocityY = 0;

    bool navingToStair = false;
    Vector3 stairTestPos = { 0,0,0 };
    Vector3 realTargetPos = { 0,0,0 };



    //Async pathfinding
    //Stores the future result of the A* path finding calculation, until that calculation finishes, we can continue running the game without waiting/blocking.
    std::future<std::vector<Vector3>> pathFuture;
    //Bool to store if we already asked for a path calc and are still waiting on it
    bool pathPending = false;
    Vector3 asyncTarget;
    const std::vector<std::vector<std::vector<bool>>>* asyncNavGrid;
    const std::vector<BoundingBox>* asyncStairs;

    //Returns smoothed 3D path, builds graph edges, A* 3D call, smooths result
    static std::vector<Vector3> ComputePath(const Vector3& start, const Vector3& target, const std::vector<std::vector<std::vector<bool>>>& navGrid, const std::vector<BoundingBox>& stairs);




    //Melee attack
    bool isAttacking = false;//True during attack animation
    float attackTimer = 0.0;//When the current attack started (if player is still too close when this timer finishes damage inflicted)
    int attackFrame = 0;//Which attack frame were on currently
    int attackFrames = 2;//How many attack frames there are
    float attackFrameTime = 0.075;//Seconds per frame
    float attackRange = 1.0f;//Distance to trigger attack
    //Update our attack logic
    void HandleAttack(const Vector3& target, Player& player);
    int attackStrenght = 25;//Defaulted here, set in spawner


    //Shooting attack
    bool isShooting = false;//True while playing shoot animation
    bool hasMeleeAttacked = false;//Dont shoot if were meleeing
    bool canMelee = true;
    bool hasLOS = false;//If LOS is currently true
    float lastSightTime = 0.0;//When we first established LOS
    float shootTimer = 0.0;//Anim frame timing
    int shotsToFire = 0;//Total consecutive shots queued up
    int shotsFired = 0;//How many consecutive shots weve done so far this loop
    int shootFrame = 0;//Which frame of the shoot anim were on
    
    //Shooting states
    enum class ShootState { Aiming, Firing };
    ShootState shootPhase = ShootState::Aiming;
    float shootStateTimer = 0.0;//State start time
    
    //Shooting vars
    float shotDamage = 6;
    int shotsToFireMin = 2;
    int shotsToFireMax = 6;

    //Update shooting logic
    void HandleShooting(const Vector3& target, const std::vector<BoundingBox>& walls, Player& player, float deltaTime);
};

#endif // ENEMY_H