#include "Player.h"

Player::Player()
{
    //Bobbing of the gun/hands while walking
    bobbingTime = 0.0f;
    bobbingSpeed = 18.0f;
    bobbingAmount = 6.0f;

    ////Player textures
    //closeToWallTexture = LoadTexture("resources/HandClose.png");//Close to wall
    //aimingShotTexture = LoadTexture("resources/AimingShot.png");//Aiming and shooting
    //aimingTexture = LoadTexture("resources/Aiming.png");//Just Aiming
    //shotTexture = LoadTexture("resources/Shot.png");//Shooting without aiming
    //idleHandTexture = LoadTexture("resources/Hand.png");//Idle hand
    ////default/idleTexture assignment
    //handTexture = idleHandTexture;
 
    //Animation bools
    isMoving = false;
    closeToWall = false;
    aiming = false;
    shot = false;
    //Animation timers
    shotTimer = 0.1f;//time it takes for shot to reset
}

void Player::HandleInput()
{
    //If the player is inputting movements then bobbing should be applied to the hand texture
    isMoving = (IsKeyDown(KEY_W) || IsKeyDown(KEY_A) || IsKeyDown(KEY_S) || IsKeyDown(KEY_D));


    //Apply the bobbing effect
    if (isMoving) bobbingTime += GetFrameTime() * bobbingSpeed;
    else bobbingTime = 0.0f;


    //Shoot gun if the left mouse btn is pressed and not up against a wall (as the gun is held in the air when up against a wall)
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !closeToWall) shot = true;
    //Timer only allows a shot to be done once 0.1 seconds or more has passed since last shot
    if (shot == true) shotTimer += GetFrameTime();
    if (shotTimer >= 0.1f)
    {
        shot = false;
        shotTimer = 0;
    }


    //Aim down the gun sight bool is true if right mouse button is pressed (in the texture assignment, the upAgainstWall animation will still take precident over aiming)
    aiming = IsMouseButtonDown(MOUSE_BUTTON_RIGHT);
}

//Wall collision calculation and animation
bool Player::calcWallCollision(Camera& camera, Color* mapPixels, Vector3 mapPosition, int mapWidth, int mapHeight, Vector3 direction)
{
    //Run animation code first as actual collision code will return to caller

    //Variables to check if player is close enough to a wall collision to enable the upAgainstWall animation
    Vector3 animTriggerPos = Vector3Add(camera.position, Vector3Scale(direction, 0.4f));//Distance of 0.4 to trigger animation
    int animTestCellX = animTriggerPos.x - mapPosition.x;
    int animTestCellY = animTriggerPos.z - mapPosition.z;

    if (animTestCellX >= 0 && animTestCellX < mapWidth && animTestCellY >= 0 && animTestCellY < mapHeight && mapPixels[animTestCellY * mapWidth + animTestCellX].b > 240)
    {
        closeToWall = true;
    }
    else
    {
        closeToWall = false;
    }



    //Variables to check if player is colliding with a wall
    Vector3 movementVector = Vector3Scale(direction, (IsKeyDown(KEY_S) ? -0.3f : 0.3f));
    Vector3 collisionTriggerPos = Vector3Add(camera.position, movementVector);//Distance of 0.3 to trigger collision, avoids clipping through walls
    collTestCellX = collisionTriggerPos.x - mapPosition.x;
    collTestCellY = collisionTriggerPos.z - mapPosition.z;

    //Collision detected
    if (collTestCellX >= 0 && collTestCellX < mapWidth && collTestCellY >= 0 && collTestCellY < mapHeight && mapPixels[collTestCellY * mapWidth + collTestCellX].b > 240)
    {
        return true;
    }
    else //No collision detected
    {
        return false;
    }
}

void Player::Animate(int screenWidth, int screenHeight, Camera& camera, Vector3 mapPosition)
{
    if (done == false)
    {
        //Player textures
        closeToWallTexture = LoadTexture("resources/HandClose.png");//Close to wall
        aimingShotTexture = LoadTexture("resources/AimingShot.png");//Aiming and shooting
        aimingTexture = LoadTexture("resources/Aiming.png");//Just Aiming
        shotTexture = LoadTexture("resources/Shot.png");//Shooting without aiming
        idleHandTexture = LoadTexture("resources/Hand.png");//Idle hand
        //default/idleTexture assignment
        handTexture = idleHandTexture;

        done = true;
    }

    //Bobbing by bobbing time by bobbing amount
    float bobbingTextureOffset = sin(bobbingTime) * bobbingAmount;
    float cropOffset = 10;//cropOffset to stop bottom of hand crop showing while texture moves up and down
    int handPosX = (screenWidth - handTexture.width) / 2;//Set to center of the screen
    int handPosY = screenHeight - handTexture.height + cropOffset + bobbingTextureOffset;//Set to center of the screen + offset for Height
    Vector2 handPos = { handPosX, handPosY };//Assign to vector2 for drawing

    //Animation assignment
    if (closeToWall) handTexture = closeToWallTexture;//Close to wall
    else if (aiming && shot) handTexture = aimingShotTexture;//Aiming and shooting
    else if (aiming) handTexture = aimingTexture;//Just Aiming
    else if (shot) handTexture = shotTexture;//Shooting without aiming
    else handTexture = idleHandTexture;//Idle hand

    //20 == map width and height values
    float scale = globals.miniMapScale;
    DrawRectangle(screenWidth - MAP_WIDTH * scale - MAP_HEIGHT + collTestCellX * scale, MAP_WIDTH + collTestCellY * scale, scale, scale, RED);//Draw players position on minimap
    DrawTextureEx(handTexture, handPos, 0.0f, 1, WHITE);//Draw hand
}