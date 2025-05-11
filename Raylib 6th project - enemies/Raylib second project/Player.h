#pragma once

//Global Libs
#include "raylib.h"
#include "raymath.h"
#include <algorithm>

//Local Libs
#include "Globals.h"

struct BulletHitResult
{
    bool hit = false;
    Vector3 point = { 0, 0, 0 };
    BoundingBox box = { 0 };
};


class Player
{

public:

    //Texture for players hand 'animations'
    Texture2D handTexture;//main texture
    //
    bool done = false;
    Texture2D closeToWallTexture;
    Texture2D aimingShotTexture;
    Texture2D aimingTexture;
    Texture2D shotTexture;
    Texture2D idleHandTexture;
    //
    bool hit33 = false;
    bool hit67 = false;
    bool hit99 = false;
    Texture2D hit33Texture;
    Texture2D hit67Texture;
    Texture2D hit99Texture;


    //Hand
    Vector2 handPos = { 0, 0 };
    //Gun textures
    Texture2D pistolTexture;
    Texture2D akTexture;
    Texture2D shotgunTexture;
    Texture2D smgTexture;
    //Ammo
    void subtractAmmo();
    void reload();
    std::pair<int, int> GetAmmo();
    int pistolAmmo = 12, pistolMaxAmmo = 12;
    int akAmmo = 30, akMaxAmmo = 30;
    int shotgunAmmo = 2, shotgunMaxAmmo = 2;
    int smgAmmo = 40, smgMaxAmmo = 40;
    //Reload
    void cancelReload();       // Stops reload on gun switch
    void updateReload();       // Called every frame
    Texture2D reloadingTexture;
    bool isReloading = false;
    float reloadTimer = 0.0f;
    const float reloadDuration = 3.0f;
    Sound pistolReload;
    Sound akReload;
    Sound shotgunReload;
    Sound smgReload;
    //Out of ammo
    Sound clickSound;
    //
    Sound painSound;





    //Weapons
    void unequipAll();
    bool pistolEquipped = true;
    bool akEquipped = false;
    bool shotgunEquipped = false;
    bool smgEquipped = false;
    //



    //Fire rates
    void setFireRate();
    float currentFireRate;
    bool justFired = false;
    float pistolFireRate = 0.5f;
    float akFireRate = 0.15f;
    float shotgunFireRate = 1.0f;
    float smgFireRate = 0.1f;
    //Shot sound
    void playSound();
    Sound pistolShot;
    Sound akShot;
    Sound shotgunShot;
    Sound smgShot;


    //Bobbing variables
    float bobbingTime;
    float bobbingSpeed;
    float bobbingAmount;
    bool isMoving;

    //Hand 'animation' bools and shot timer (in order of importance, ie top ones overwrite bottom ones, closeToWall 'animation' overwrites all other animations)
    bool closeToWall;//Brings hands and gun closer on near wall collision
    bool closeToWallZ;//Brings hands and gun closer on near wall collision
    bool aiming;//Aim down sight
    bool shot;//Shoot gun, if aiming down sight, shooting while aiming activates instead using the same bool
    //Timer for 'animations'
    float shotTimer;// 0.1 seconds timer, for the time it takes for shot to reset

    //Player world position
    int collTestCellX;
    int collTestCellY;
    Vector3 position = { 10, 0.5f, 10 };
    int layer = 0;
    //Player hitbox
    BoundingBox hitbox;
    float hitBoxWidth = 0.23f;
    float hitBoxHeight = 0.4f;


    //Function to handle bounding box collision and restrict movement
    void PreventBoundingBoxCollisions(const std::vector<BoundingBox>& obstacles, BoundingBox& playerBox, Camera& camera, Vector3 oldCamPos);
    void PreventBoundingBoxCollision(const BoundingBox obstacle, BoundingBox& playerBox, Camera& camera, Vector3 oldCamPos);
    //Function to handle close to wall animation just before movement is restricted
    void closeToWallCheck(Camera& camera, const std::vector<BoundingBox>& walls);


public:

    Player();

    void update(Camera camera);

    //Globals globals;//Giving player access to globals

    void HandleInput();//Function handles all player controlls for animations.
    void Animate(int screenWidth, int screenHeight, Camera& camera, Vector3 mapPosition);//Draws the players hand on the screen (function called in main) 

    Vector3 calcBulletCollision(Camera& camera, BoundingBox boxCollider);//Returns position of closest hit bounding box
    BulletHitResult getBulletCollision(Camera& camera, const std::vector<BoundingBox>& boxes);//Returns position of closest hit bounding box

    //Players shooting/bullets
    Vector3 hitPoint = { 0 };
    bool hitTarget = false;



    /// Physcics (gravity, ground detection and stepping up)
    //Gravity vars
    float velocityY = 0.0f;//Current vertical velocity
    bool onGround = false;//True if feet are supported this frame
    float gravitySpeed = -20.0f;//Gravity speed buildup until it reaches max speed
    float gravityMax = -25.0f;//Clamped max fall speed
	float threshold = 0.02f;//stops oscillation when close to ground and not in a perfect position
    bool doGravity = false;//Dont activate gravity until after first loop (must init without it to avoid issues)

	//Funcs
    void ApplyGravity(Camera& cam, BoundingBox& playerBox, const std::vector<BoundingBox>& obstacles);
    //If the player walks into a slight step, move them up onto it (stairs, slopes)
    bool TryStepUp(float deltaY, Camera& camera, BoundingBox& playerBox, const std::vector<BoundingBox>& obstacles);
    //Ground detection, prevents player from falling through the floor
    void GroundCollisions(Camera& camera, BoundingBox& playerBox, const std::vector<BoundingBox>& obstacles, float maxClimb = 1.0f, float stepThreshold = 0.01f);



    //New Movement and Looking
    void MoveAndCollide(float deltaTime, Camera& camera, const std::vector<BoundingBox>& obstacles, const std::vector<BoundingBox>& doors);
    float walkSpeed = 4.0f;
    float sprintMultiplier = 1.38f;
    bool isSprinting;

    //Acceleration and deceleration
    float currentSpeed = 0.0f;//Current
    float accelRate = 20.0f;//Speeding up
    float decelRate = 25.0f;//Slowing down
    

    //Looking around
    void UpdateLookAngles();
    void ApplyLook(Camera& camera);
    float yaw = -90.0f;//0 faces +X, -90 faces -Z
    float pitch = 0.0f;
    float mouseSensitivity = 0.15f;
    Vector3 front = { 0,0,-1 };

    //FOV
    void UpdateFOV(Camera& camera, float deltaTime);
    float baseFOV = 45.0f;
    float maxFOV = 50.0f;
    float currentFOV = baseFOV;
    float fovChangeRate = 60.0f;//Degrees per second change


    void takeDamage(float damageAmount);
    int health = 100;
    float invulnerabilityTimer;
    float invulnerabilityDuration = 0.5f;
};