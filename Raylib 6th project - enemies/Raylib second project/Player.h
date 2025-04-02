#pragma once

//Global Libs
#include "raylib.h"
#include "raymath.h"

//Local Libs
#include "Globals.h"

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

    //Hand
	Vector2 handPos = { 0, 0 };
	//Gun textures
	Texture2D pistolTexture;
	Texture2D akTexture;
	Texture2D shotgunTexture;
	Texture2D smgTexture;

    //Weapons
	void unequipAll();
    bool pistolEquipped = true;
	bool akEquipped = false;
	bool shotgunEquipped = false;
	bool smgEquipped = false;

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
    //Player hitbox
    BoundingBox hitbox;
    float hitBoxWidth = 0.23f;
    float hitBoxHeight = 0.5f;
	

    //Function to handle bounding box collision and restrict movement
    void PreventBoundingBoxCollisions(const std::vector<BoundingBox>& obstacles, BoundingBox& playerBox, Camera& camera, Vector3 oldCamPos);
    void PreventBoundingBoxCollision(const BoundingBox obstacle, BoundingBox& playerBox, Camera& camera, Vector3 oldCamPos);
    //Function to handle close to wall animation just before movement is restricted
	void closeToWallCheck(Camera& camera, const std::vector<BoundingBox>& walls);


public:

    Player();

    void update(Camera camera);

    Globals globals;//Giving player access to globals

    void HandleInput();//Function handles all player controlls for animations.
    void Animate(int screenWidth, int screenHeight, Camera& camera, Vector3 mapPosition);//Draws the players hand on the screen (function called in main) 

	Vector3 calcBulletCollision(Camera& camera, BoundingBox boxCollider);//Returns position of closest hit bounding box

    //Players shooting/bullets
    Vector3 hitPoint = { 0 };
    bool hitTarget = false;
};