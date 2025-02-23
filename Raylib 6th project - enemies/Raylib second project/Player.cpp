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
    closeToWallX = false;
    closeToWallZ = false;
    aiming = false;
    shot = false;
    //Animation timers
    shotTimer = 0.1f;//time it takes for shot to reset

	//Player hitbox
    hitbox.min = { position.x - hitBoxWidth, position.y - hitBoxHeight, position.z - hitBoxWidth };
    hitbox.max = { position.x + hitBoxWidth, position.y + hitBoxHeight, position.z + hitBoxWidth };


}

void Player::update(Camera camera)
{
	position = camera.position;
}

void Player::HandleInput()
{
    //If the player is inputting movements then bobbing should be applied to the hand texture
    isMoving = (IsKeyDown(KEY_W) || IsKeyDown(KEY_A) || IsKeyDown(KEY_S) || IsKeyDown(KEY_D));


    //Apply the bobbing effect
    if (isMoving) bobbingTime += GetFrameTime() * bobbingSpeed;
    else bobbingTime = 0.0f;


    //Shoot gun if the left mouse btn is pressed and not up against a wall (as the gun is held in the air when up against a wall)
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !closeToWallX && !closeToWallZ) shot = true;
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
    //Variables to check if player is close enough to a wall collision to enable the upAgainstWall animation
    Vector3 animTriggerPos = Vector3Add(camera.position, Vector3Scale(direction, 0.4f));//Distance of 0.4 to trigger animation
    int animTestCellX = animTriggerPos.x - mapPosition.x;
    int animTestCellY = animTriggerPos.z - mapPosition.z;

    if (animTestCellX >= 0 && animTestCellX < mapWidth && animTestCellY >= 0 && animTestCellY < mapHeight && mapPixels[animTestCellY * mapWidth + animTestCellX].b > 240)
    {
        closeToWallX = true;
    }
    else
    {
        closeToWallX = false;
    }

    return 1;
}

Vector3 Player::calcBulletCollision(Camera& camera, BoundingBox boxCollider)
{
    //Bullet ray
    Vector3 rayDirection = Vector3Normalize(Vector3Subtract(camera.target, camera.position));
    Vector3 rayLinePoint;

    //Iterate along the array to detect collision
    for (float t = 0; t < 50.0f; t += 0.05f)
    {
        rayLinePoint = Vector3Add(camera.position, Vector3Scale(rayDirection, t));

        //Check if the point is within the bounding box
        if (rayLinePoint.x >= boxCollider.min.x && rayLinePoint.x <= boxCollider.max.x &&
            rayLinePoint.y >= boxCollider.min.y && rayLinePoint.y <= boxCollider.max.y &&
            rayLinePoint.z >= boxCollider.min.z && rayLinePoint.z <= boxCollider.max.z)
        {
            return rayLinePoint;//Return the collision point
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
    if (closeToWallX or closeToWallZ) handTexture = closeToWallTexture;//Close to wall
    else if (aiming && shot) handTexture = aimingShotTexture;//Aiming and shooting
    else if (aiming) handTexture = aimingTexture;//Just Aiming
    else if (shot) handTexture = shotTexture;//Shooting without aiming
    else handTexture = idleHandTexture;//Idle hand

    //20 == map width and height values
    float scale = globals.miniMapScale / 2;
    DrawCircle(screenWidth - MAP_WIDTH * scale - MAP_HEIGHT + position.x * scale, MAP_WIDTH + position.z * scale, scale, GREEN);//Draw players position on minimap
    DrawTextureEx(handTexture, handPos, 0.0f, 1, WHITE);//Draw hand
}




void Player::PreventBoundingBoxCollisions(const std::vector<BoundingBox>& obstacles, BoundingBox& playerBox, Camera& camera, Vector3 oldCamPos)
{
    //Position the camera wants to be after moving
    Vector3 desiredPos = camera.position;
    //Calculates how far the camera moved in total
    Vector3 totalMovement = Vector3Subtract(desiredPos, oldCamPos);


    /// /// /// Applies movement along the X axis /// /// ///
    camera.position.x = oldCamPos.x + totalMovement.x;

    //Updates players bounding box for attempted X axis movement
    playerBox.min.x = camera.position.x - hitBoxWidth;
    playerBox.max.x = camera.position.x + hitBoxWidth;

    //Checks if this new X position collides with any obstacle
    for (const auto& box : obstacles)
    {
        if (CheckCollisionBoxes(playerBox, box))
        {
            //Collided on X axis  --  revert X movement
            camera.position.x = oldCamPos.x;
            closeToWallX = true;

            //Update bounding box back to old X pos
            playerBox.min.x = camera.position.x - hitBoxWidth;
            playerBox.max.x = camera.position.x + hitBoxWidth;
            break;
        }
        else closeToWallX = false;
    }

    /// /// /// Applies movement along the Z axis /// /// ///
    camera.position.z = oldCamPos.z + totalMovement.z;

    //Updates bounding box for attempted Z axismovement
    playerBox.min.z = camera.position.z - hitBoxWidth;
    playerBox.max.z = camera.position.z + hitBoxWidth;

    //Checks if this new Z position collides with any obstacle
    for (const auto& box : obstacles)
    {
        if (CheckCollisionBoxes(playerBox, box))
        {
            //Collided on Z axis  --  revert Z movement
            camera.position.z = oldCamPos.z;
            closeToWallZ = true;

            //Update bounding box back to old Z pos
            playerBox.min.z = camera.position.z - hitBoxWidth;
            playerBox.max.z = camera.position.z + hitBoxWidth;
            break;
        }
		else closeToWallZ = false;
    }

	//Update bounding box for the final position
    playerBox.min = { camera.position.x - hitBoxWidth, camera.position.y - hitBoxHeight - 0.1f, camera.position.z - hitBoxWidth };
    playerBox.max = { camera.position.x + hitBoxWidth, camera.position.y + hitBoxHeight - 0.1f,  camera.position.z + hitBoxWidth };
}






void Player::PreventBoundingBoxCollision(const BoundingBox obstacle, BoundingBox& playerBox, Camera& camera, Vector3 oldCamPos)
{
    //Position the camera wants to be after moving
    Vector3 desiredPos = camera.position;
    //Calculates how far the camera moved in total
    Vector3 totalMovement = Vector3Subtract(desiredPos, oldCamPos);


    /// /// /// Applies movement along the X axis /// /// ///
    camera.position.x = oldCamPos.x + totalMovement.x;

    //Updates players bounding box for attempted X axis movement
    playerBox.min.x = camera.position.x - hitBoxWidth;
    playerBox.max.x = camera.position.x + hitBoxWidth;

    //Checks if this new X position collides with any obstacle

    if (CheckCollisionBoxes(playerBox, obstacle))
    {
        //Collided on X axis  --  revert X movement
        camera.position.x = oldCamPos.x;

        //Update bounding box back to old X pos
        playerBox.min.x = camera.position.x - hitBoxWidth;
        playerBox.max.x = camera.position.x + hitBoxWidth;
    }
    

    /// /// /// Applies movement along the Z axis /// /// ///
    camera.position.z = oldCamPos.z + totalMovement.z;

    //Updates bounding box for attempted Z axismovement
    playerBox.min.z = camera.position.z - hitBoxWidth;
    playerBox.max.z = camera.position.z + hitBoxWidth;

    //Checks if this new Z position collides with any obstacle

    if (CheckCollisionBoxes(playerBox, obstacle))
    {
        //Collided on Z axis  --  revert Z movement
        camera.position.z = oldCamPos.z;

        //Update bounding box back to old Z pos
        playerBox.min.z = camera.position.z - hitBoxWidth;
        playerBox.max.z = camera.position.z + hitBoxWidth;
    }
    


    //Update bounding box for the final position
    playerBox.min = { camera.position.x - hitBoxWidth, camera.position.y - hitBoxHeight - 0.1f, camera.position.z - hitBoxWidth };
    playerBox.max = { camera.position.x + hitBoxWidth, camera.position.y + hitBoxHeight - 0.1f,  camera.position.z + hitBoxWidth };
}