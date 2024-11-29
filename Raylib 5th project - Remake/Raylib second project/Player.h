#pragma once

//Global Libs
#include "raylib.h"
#include "raymath.h"

//Local Libs
#include "Globals.h"

//Globals globals;

class Player
{

private:

    //Texture for players hand 'animations'
    Texture2D handTexture;//main texture
    //
    bool done = false;
    Texture2D closeToWallTexture;
    Texture2D aimingShotTexture;
    Texture2D aimingTexture;
    Texture2D shotTexture;
    Texture2D idleHandTexture;


    //Bobbing variables
    float bobbingTime;
    float bobbingSpeed;
    float bobbingAmount;
    bool isMoving;

    //Hand 'animation' bools and shot timer (in order of importance, ie top ones overwrite bottom ones, closeToWall 'animation' overwrites all other animations)
    bool closeToWall;//Brings hands and gun closer on near wall collision
    bool aiming;//Aim down sight
    bool shot;//Shoot gun, if aiming down sight, shooting while aiming activates instead using the same bool
    //Timer for 'animations'
    float shotTimer;// 0.1 seconds timer, for the time it takes for shot to reset

    //Player world position
    int collTestCellX;
    int collTestCellY;

public:
    Player();

    Globals globals;//Giving player access to globals

    void HandleInput();//Function handles all player controlls for animations.
    void Animate(int screenWidth, int screenHeight, Camera& camera, Vector3 mapPosition);//Draws the players hand on the screen (function called in main) 
    bool calcWallCollision(Camera& camera, Color* mapPixels, Vector3 mapPosition, int mapWidth, int mapHeight, Vector3 direction);//Checks wall collision for movement and to enable closeToWall anim
};