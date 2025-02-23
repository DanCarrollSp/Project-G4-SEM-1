#include "Player.h"
#include <iostream>

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
    //Bullet collisions
    Vector3 rayDirection = Vector3Normalize(Vector3Subtract(camera.target, camera.position));

    //Iterate along the array to detect collision
    for (float t = 0; t < 50.0f; t += 0.1f)
    {
        Vector3 checkPoint = Vector3Add(camera.position, Vector3Scale(rayDirection, t));
        int cellX = (int)(checkPoint.x - mapPosition.x);
        int cellY = (int)(checkPoint.z - mapPosition.z);

        if (cellX >= 0 && cellX < MAP_WIDTH && cellY >= 0 && cellY < MAP_HEIGHT)
        {
            Color pixelColor = mapPixels[cellY * MAP_WIDTH + cellX];

            //Check if the ray hits anything other than open spaces (BLACK == open spaces)
            if ((pixelColor.r == WHITE.r && pixelColor.g == WHITE.g && pixelColor.b == WHITE.b) or
                (pixelColor.r == BLUE.r && pixelColor.g == BLUE.g && pixelColor.b == BLUE.b))
            {
                hitTarget = true;
                hitPoint = checkPoint;
                break;
            }
        }
    }





    /// <--> Run animation code first as actual collision code will return to caller

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

Vector3 Player::calcBulletCollision(Camera& camera, BoundingBox boxCollider)
{
    //Bullet ray
    Vector3 rayDirection = Vector3Normalize(Vector3Subtract(camera.target, camera.position));
    Vector3 checkPoint;

    //Iterate along the array to detect collision
    for (float t = 0; t < 50.0f; t += 0.1f)
    {
        checkPoint = Vector3Add(camera.position, Vector3Scale(rayDirection, t));

        //Check if the point is within the bounding box
        if (checkPoint.x >= boxCollider.min.x && checkPoint.x <= boxCollider.max.x &&
            checkPoint.y >= boxCollider.min.y && checkPoint.y <= boxCollider.max.y &&
            checkPoint.z >= boxCollider.min.z && checkPoint.z <= boxCollider.max.z)
        {
            return checkPoint;//Return the collision point
        }
    }

    //No collision found, return (0,0,0)
    return { 0.0f, 0.0f, 0.0f };
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

    //Bobbing hand by bobbing time by bobbing amount
    Vector2 centerOffset = { 57.5 , 10};//Offset to put the gun dead in the center of the screen for shooting
    float bobbingTextureOffset = sin(bobbingTime) * bobbingAmount;
    float cropOffset = 10;//cropOffset to stop bottom of hand crop showing while texture moves up and down
    int handPosX = (screenWidth - handTexture.width - centerOffset.x) / 2;//Set to center of the screen
    int handPosY = screenHeight - handTexture.height + cropOffset + bobbingTextureOffset + centerOffset.y;//Set to center of the screen + offset for Height
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