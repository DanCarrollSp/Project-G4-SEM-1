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

	//Player hitbox
    hitbox.min = { position.x - hitBoxWidth, position.y - hitBoxHeight, position.z - hitBoxWidth };
    hitbox.max = { position.x + hitBoxWidth, position.y + hitBoxHeight, position.z + hitBoxWidth };
}

void Player::update(Camera camera)
{
	setFireRate();

	position = camera.position;
    //Update bounding box for the final position
    hitbox.min = { camera.position.x - hitBoxWidth, camera.position.y - hitBoxHeight - 0.1f, camera.position.z - hitBoxWidth };
    hitbox.max = { camera.position.x + hitBoxWidth, camera.position.y + hitBoxHeight - 0.1f,  camera.position.z + hitBoxWidth };

    //change guns
    if (IsKeyPressed(KEY_ONE)) { unequipAll(); pistolEquipped = true; cancelReload(); }
    if (IsKeyPressed(KEY_TWO)) { unequipAll(); akEquipped = true; cancelReload(); }
    if (IsKeyPressed(KEY_THREE)) { unequipAll(); shotgunEquipped = true; cancelReload(); }
    if (IsKeyPressed(KEY_FOUR)) { unequipAll(); smgEquipped = true; cancelReload(); }

}

void Player::HandleInput()
{
    //If the player is inputting movements then bobbing should be applied to the hand texture
    isMoving = (IsKeyDown(KEY_W) || IsKeyDown(KEY_A) || IsKeyDown(KEY_S) || IsKeyDown(KEY_D));


    //Apply the bobbing effect
    if (isMoving) bobbingTime += GetFrameTime() * bobbingSpeed;
    else bobbingTime = 0.0f;



    //If the player has just fired, set the justFired bool to true
    // Increases the shot cooldown timer every frame
        shotTimer += GetFrameTime();
        justFired = false;

        if (!isReloading)
        {
            bool shouldShoot = false;

            // Semi-auto weapons use mouse click, no cooldown
            if ((pistolEquipped || shotgunEquipped) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                shouldShoot = true;
            }
            // Full-auto weapons use hold + cooldown
            else if ((akEquipped || smgEquipped) && IsMouseButtonDown(MOUSE_BUTTON_LEFT) && shotTimer >= currentFireRate)
            {
                shouldShoot = true;
            }

            if (shouldShoot)
            {
                bool hasAmmo = false;

                if (pistolEquipped && pistolAmmo > 0) hasAmmo = true;
                if (akEquipped && akAmmo > 0) hasAmmo = true;
                if (shotgunEquipped && shotgunAmmo > 0) hasAmmo = true;
                if (smgEquipped && smgAmmo > 0) hasAmmo = true;

                if (hasAmmo)
                {
                    justFired = true;
                    shotTimer = 0.0f;
                    playSound();
                    subtractAmmo();
                }
                else
                {
                    PlaySound(clickSound);
                    shotTimer = 0.0f;
                }
            }

            //Reload
            if (IsKeyPressed(KEY_R)) reload();

        }

        //Reload with cancable reload
        //if (IsKeyPressed(KEY_R))
        //{
        //    if (isReloading) cancelReload();
        //    else  reload();
        //}

        updateReload();


    //Aim down the gun sight bool is true if right mouse button is pressed (in the texture assignment, the upAgainstWall animation will still take precident over aiming)
    //aiming = IsMouseButtonDown(MOUSE_BUTTON_RIGHT);
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

BulletHitResult Player::getBulletCollision(Camera& camera, const std::vector<BoundingBox>& boxes)
{
    BulletHitResult result;

    Vector3 rayOrigin = camera.position;
    Vector3 rayDirection = Vector3Normalize(Vector3Subtract(camera.target, camera.position));
    Ray ray = { rayOrigin, rayDirection };

    float closestDistance = FLT_MAX;

    for (const auto& box : boxes)
    {
        RayCollision collision = GetRayCollisionBox(ray, box);

        if (collision.hit && collision.distance < closestDistance)
        {
            closestDistance = collision.distance;
            result.hit = true;
            result.point = collision.point;
            result.box = box;
        }
    }

    return result;
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
        idleHandTexture = LoadTexture("resources/Guns/Pistol.png");//Idle hand
        //default/idleTexture assignment
        handTexture = idleHandTexture;


		//Gun textures
		pistolTexture = LoadTexture("resources/Guns/Pistol.png");
		akTexture = LoadTexture("resources/Guns/Ak.png");
		shotgunTexture = LoadTexture("resources/Guns/Shotgun.png");
		smgTexture = LoadTexture("resources/Guns/Smg.png");
        //Gun sounds
        pistolShot = LoadSound("resources/Sounds/pistolShot.wav");
        akShot = LoadSound("resources/Sounds/akShot.wav");
        shotgunShot = LoadSound("resources/Sounds/shotgunShot.wav");
        smgShot = LoadSound("resources/Sounds/smgShot.wav");

        //
        pistolReload = LoadSound("resources/Sounds/pistolReload.wav");
        akReload = LoadSound("resources/Sounds/akReload.wav");
        shotgunReload = LoadSound("resources/Sounds/shotgunReload.wav");
        smgReload = LoadSound("resources/Sounds/smgReload.wav");
        reloadingTexture = LoadTexture("resources/Guns/reloading.png");
        clickSound = LoadSound("resources/Sounds/Click.wav");


        done = true;
    }

    //Bobbing hand by bobbing time by bobbing amount
    if (justFired)
    {
        Vector2 centerOffset = { 57.5 , 10 };//Offset to put the gun dead in the center of the screen for shooting
        float cropOffset = 10;//cropOffset to stop bottom of hand crop showing while texture moves up and down
        handPos.x = ((screenWidth - handTexture.width - centerOffset.x) / 2) + 15;//Set to center of the screen
        handPos.y = screenHeight - handTexture.height + cropOffset + centerOffset.y + 10;//Set to center of the screen + offset for Height
	}
	else
    {
        Vector2 centerOffset = { 57.5 , 10 };//Offset to put the gun dead in the center of the screen for shooting
        float bobbingTextureOffset = sin(bobbingTime) * bobbingAmount;
        float cropOffset = 10;//cropOffset to stop bottom of hand crop showing while texture moves up and down
        handPos.x = (screenWidth - handTexture.width - centerOffset.x) / 2;//Set to center of the screen
        handPos.y = screenHeight - handTexture.height + cropOffset + bobbingTextureOffset + centerOffset.y;//Set to center of the screen + offset for Height
        
    }

    //Animation assignment
    //if (closeToWall) handTexture = closeToWallTexture;//Close to wall
    //else if (aiming && shot) handTexture = aimingShotTexture;//Aiming and shooting
    //else if (aiming) handTexture = aimingTexture;//Just Aiming
    //else if (shot) handTexture = shotTexture;//Shooting without aiming
    //else handTexture = idleHandTexture;//Idle hand

	if (pistolEquipped) handTexture = pistolTexture;
	else if (akEquipped) handTexture = akTexture;
	else if (shotgunEquipped) handTexture = shotgunTexture;
	else if (smgEquipped) handTexture = smgTexture;



	//Draws the players hand and updates its animations
    if (isReloading)
    {
        //Makes it pulse
        float pulse = 0.8f + 0.012f * sin(reloadTimer * 6.0f); // Pulses 3 times per second

        // Calculates scaled size
        float width = reloadingTexture.width * pulse;
        float height = reloadingTexture.height * pulse;

        // Centers the icon
        Vector2 iconPos = {
            (float)screenWidth / 2 - width / 2,
            (float)screenHeight - height - 60
        };

        // Draw with scaling
        DrawTextureEx(reloadingTexture, iconPos, 0.0f, pulse, WHITE);
    }
    else
    {
        DrawTextureEx(handTexture, handPos, 0.0f, 1, WHITE);
    }

}




std::pair<int, int> Player::GetAmmo()
{
    if (pistolEquipped) return { pistolAmmo, pistolMaxAmmo };
    if (akEquipped) return { akAmmo, akMaxAmmo };
    if (shotgunEquipped) return { shotgunAmmo, shotgunMaxAmmo };
    if (smgEquipped) return { smgAmmo, smgMaxAmmo };
    return { 0, 0 };
}


void Player::unequipAll()
{
	pistolEquipped = false;
	akEquipped = false;
	shotgunEquipped = false;
	smgEquipped = false;
}

void Player::setFireRate()
{
	if (pistolEquipped) currentFireRate = pistolFireRate;
	else if (akEquipped) currentFireRate = akFireRate;
	else if (shotgunEquipped) currentFireRate = shotgunFireRate;
	else if (smgEquipped) currentFireRate = smgFireRate;
}

void Player::playSound()
{
	if (pistolEquipped) PlaySound(pistolShot);
	else if (akEquipped) PlaySound(akShot);
	else if (shotgunEquipped) PlaySound(shotgunShot);
	else if (smgEquipped) PlaySound(smgShot);
}

void Player::PreventBoundingBoxCollisions(const std::vector<BoundingBox>& obstacles, BoundingBox& playerBox, Camera& camera, Vector3 oldCamPos)
{
    //Applies gravity and checks ground collisions
    if (doGravity)ApplyGravity(camera, playerBox, obstacles);
    GroundCollisions(camera, playerBox, obstacles);
    doGravity = true;

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
            //Check if its climbable first
            float deltaY = box.max.y - playerBox.min.y;

			//Max step up height
            const float maxStep = 0.6f;

            if (deltaY > 0.0f && deltaY <= maxStep && TryStepUp(deltaY + 0.01f, camera, playerBox, obstacles))
            {
				//Stepped up succesfully, keep the new X and Y pos
            }
            else
            {
                //Cant step over so just collide with it face first
                camera.position.x = oldCamPos.x;
                playerBox.min.x = camera.position.x - hitBoxWidth;
                playerBox.max.x = camera.position.x + hitBoxWidth;
            }
        }

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

            //Update bounding box back to old Z pos
            playerBox.min.z = camera.position.z - hitBoxWidth;
            playerBox.max.z = camera.position.z + hitBoxWidth;
            break;
        }
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

void Player::closeToWallCheck(Camera& camera, const std::vector<BoundingBox>& walls)
{
    //Defines the ray starting at the camera position pointing towards camera.target
    Ray ray;
    ray.position = camera.position;
    ray.direction = Vector3Normalize(Vector3Subtract(camera.target, camera.position));
	float checkDistance = 0.45f;//Distance to check for wall collision

	//Defautl to false
    closeToWall = false;

    //Checks through each walls bounding box
    for (const auto& wall : walls)
    {
        //Check for a ray collision
        RayCollision collision = GetRayCollisionBox(ray, wall);
		//Set to true if collision found
        if (collision.hit && (collision.distance <= checkDistance))
        {
            closeToWall = true;
			break;//No need to check further so break out of the loop
        }
    }
}


void Player::subtractAmmo()
{
    if (pistolEquipped && pistolAmmo > 0) pistolAmmo--;
    else if (akEquipped && akAmmo > 0) akAmmo--;
    else if (shotgunEquipped && shotgunAmmo > 0) shotgunAmmo--;
    else if (smgEquipped && smgAmmo > 0) smgAmmo--;
}

void Player::reload()
{
    isReloading = true;
    reloadTimer = 0.0f;
    if (pistolEquipped) PlaySound(pistolReload);
    else if (akEquipped) PlaySound(akReload);
    else if (shotgunEquipped) PlaySound(shotgunReload);
    else if (smgEquipped) PlaySound(smgReload);
}

void Player::cancelReload()
{
    isReloading = false;
    reloadTimer = 0.0f;

    StopSound(pistolReload);
    StopSound(akReload);
    StopSound(shotgunReload);
    StopSound(smgReload);
}

void Player::updateReload()
{
    if (isReloading)
    {
        reloadTimer += GetFrameTime();
        float totalReloadTime = reloadDuration;

        if (shotgunEquipped or pistolEquipped)
        {
            totalReloadTime += 1.0f; // Add 1 extra seconds for shotgun and pistol
        }

        if (reloadTimer >= totalReloadTime)
        {
            if (pistolEquipped) pistolAmmo = pistolMaxAmmo;
            else if (akEquipped) akAmmo = akMaxAmmo;
            else if (shotgunEquipped) shotgunAmmo = shotgunMaxAmmo;
            else if (smgEquipped) smgAmmo = smgMaxAmmo;

            isReloading = false;
        }

    }
}



/// ///////////////////////////////////////////////////////////////////////////////////////////
//Physics for the player (ground collisions, gravity, stepping up)

//Step up onto colliders if they are short enough (Stairs etc)
bool Player::TryStepUp(float deltaY, Camera& camera, BoundingBox& playerBox, const std::vector<BoundingBox>& obstacles)
{
	//Camera and collider positions
    camera.position.y += deltaY;
    playerBox.min.y += deltaY;
    playerBox.max.y += deltaY;

    for (const auto& obstacle : obstacles)
        if (CheckCollisionBoxes(playerBox, obstacle))
        {
            //Dont allow step up
            camera.position.y -= deltaY;
            playerBox.min.y -= deltaY;
            playerBox.max.y -= deltaY;
            return false;
        }

	//Low enough step up
    return true;
}

void Player::GroundCollisions(Camera& camera, BoundingBox& playerBox, const std::vector<BoundingBox>& obstacles, float maxClimb, float stepThreshold)
{
    //Init climbed as 0 as no climbing has been decided yet
    float climbed = 0.0f;

    //Loops until no collision or we exceeded the climb height allowed
    while (climbed < maxClimb)
    {
        //Finds the smallest positive lift that would place the playerBox exactly on top of the collider
        float minDy = FLT_MAX;
        for (const auto& box : obstacles)
        {
            //If we are colliding with a box, check if its height is low enough to see if we can step up onto it
            if (CheckCollisionBoxes(playerBox, box))
            {
                float dy = box.max.y - playerBox.min.y;
                if (dy > 0.0f && dy < minDy) minDy = dy;
            }
        }


        if (minDy == FLT_MAX) break;//No overlapping of player box and obstacle boxes = no climbing
		if (!TryStepUp(minDy + stepThreshold, camera, playerBox, obstacles)) break;//Step up failed = no climbing

        climbed += minDy + stepThreshold;//Passed checks, climb
    }

	//Once climbing is succesful, reset velocity as we are now on the ground
    if (climbed > 0.0f)
    {
		velocityY = 0.0f;//Stop any falling
        onGround = true;//Stop updating any falling
    }
}

//Applies gravity to the player if they are not on the ground in the form of downward velocity and resolves floor collisions
// Applies gravity to the player by increasing downward velocity and resolving collisions
void Player::ApplyGravity(Camera& camera, BoundingBox& playerBounds, const std::vector<BoundingBox>& obstacles)
{
    //Time elapsed since last frame
    float deltaTime = GetFrameTime();

    //Accumulate gravity acceleration
    velocityY += gravitySpeed * deltaTime;
    //Clamp to max gravity speed
    if (velocityY < gravityMax) velocityY = gravityMax;


    //Gets total downward movement for this frame
    float totalFallDistance = velocityY * deltaTime;
    //Max downward movement allowed before a check to avoid tunneling through thin obstacles
    const float checkFallStepAmount = -0.45f;
    //Remaining distance to sweep after that step
    float remainingDistance = totalFallDistance;


    //Assume we are falling until collision says were not
    onGround = false;


    //Sweep down in small increments until weve moved full distance or landed (increments of checkFallStepAmount)
    while (remainingDistance < 0.0f)
    {
        //Step down either the max allowed per loop, or whatevers left of the full distance to fall remaining
        float stepDistance = std::max(remainingDistance, checkFallStepAmount);

        //Move camera and bounding box down
        camera.position.y += stepDistance;
        playerBounds.min.y += stepDistance;
        playerBounds.max.y += stepDistance;

		//Take away how much weve moved down from how we have left to move down
        remainingDistance -= stepDistance;


        //Checks for collision with any obstacle
        bool hasLanded = false;//assume we havent landed until we do the calc
        for (const auto& obstacle : obstacles)
        {
			//If were not colliding with the obstacle, skip to the next one
            if (!CheckCollisionBoxes(playerBounds, obstacle)) continue;

            //Snap player to the top of that obstacle
            float obstacleTopY = obstacle.max.y;
            float footOffset = hitBoxHeight + threshold;
            //
            camera.position.y = obstacleTopY + footOffset;
            playerBounds.min.y = obstacleTopY + threshold;
            playerBounds.max.y = obstacleTopY + footOffset * 2;

            //Stop falling
            velocityY = 0.0f;
            onGround = true;
            hasLanded = true;
            break;
        }

        //If weve landed, exit the loop
        if (hasLanded) break;
    }
}