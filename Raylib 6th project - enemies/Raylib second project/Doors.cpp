//Doors.cpp
#include "Doors.h"

//Taken from Main
extern Camera camera;
extern Color* mapPixels;
extern World  world;
extern Player player;


//Movement offsets for each door type
float
DOOR1_DX = 0.0f, DOOR1_DZ = -0.9f,//Normal Doors
DOOR2_DX = +0.9f, DOOR2_DZ = 0.0f,
DOOR_BIG_LEFT_DX1 = -0.9f, DOOR_BIG_LEFT_DZ1 = 0.0f,//Big Doors
DOOR_BIG_RIGHT_DX1 = +0.9f, DOOR_BIG_RIGHT_DZ1 = 0.0f,
DOOR_BIG_LEFT_DX2 = 0.0f, DOOR_BIG_LEFT_DZ2 = +0.9f,
DOOR_BIG_RIGHT_DX2 = 0.0f, DOOR_BIG_RIGHT_DZ2 = -0.9f;

//Apply member variables to all our doors made in the level
std::vector<DoorConfig> doors;
bool initialized = false;

void InitDoors()
{
    //Clear out and refill all stored doors
    auto& boxes = world.GetDoorBoundingBoxes();
    doors.clear();
    doors.reserve(boxes.size());

    //Find out which door it is in the world, block it from being drawn normally, add it to our new storage of doors and apply its opened/closed/progress status (then draw later)
    for (auto& box : boxes)
    {
        //At the doors position on the map see what color it is so well know what kind of door it is
        int xPos = (int)floorf(box.min.x);
        int yPos = (int)floorf(box.min.y);
        int zPos = (int)floorf(box.min.z);
        int index = (yPos * MAP_LENGHT + zPos) * MAP_WIDTH + xPos;
        Color color = mapPixels[index];

        //Now we know what door it is, hide it
        mapPixels[index] = BLACK;


        //Decide what texture it needs and which way itll move
        float destX = 0;
        float destZ = 0;
        Texture2D texture = TEST_TEXTURE;//Default to test text to visualise issues

        if (ColorEq(color, DoorColor1))//
        {
            //Small door
            destX = DOOR1_DX;
            destZ = DOOR1_DZ;
            texture = TextureDoor;
        }
        else if (ColorEq(color, DoorColor2))
        {
            //Small door (different movement)
            destX = DOOR2_DX;
            destZ = DOOR2_DZ;
            texture = TextureDoor;
        }

        else if (ColorEq(color, BigDoorLeftColor1))//
        {
            //Left half of the big sliding door
            destX = DOOR_BIG_LEFT_DX1;
            destZ = DOOR_BIG_LEFT_DZ1;
            texture = TextureBigDoorLeft;
        }
        else if (ColorEq(color, BigDoorRightColor1))
        {
            //Right half of the big sliding door
            destX = DOOR_BIG_RIGHT_DX1;
            destZ = DOOR_BIG_RIGHT_DZ1;
            texture = TextureBigDoorRight;
        }
        else if (ColorEq(color, BigDoorLeftColor2))//
        {
            //Left half of the big sliding door (different movement)
            destX = DOOR_BIG_LEFT_DX2;
            destZ = DOOR_BIG_LEFT_DZ2;
            texture = TextureBigDoorLeft;
        }
        else if (ColorEq(color, BigDoorRightColor2))
        {
            //Right half of the big sliding door
            destX = DOOR_BIG_RIGHT_DX2;
            destZ = DOOR_BIG_RIGHT_DZ2;
            texture = TextureBigDoorRight;
        }
        else
        {
            //Color not caught, skip setting up the door
            continue;
        }


        //Set up the doors movement, bounding box and texture
        DoorConfig doorConfig;
        doorConfig.box = &box;
        doorConfig.closedMin = box.min;
        doorConfig.closedMax = box.max;
        doorConfig.openMin = { box.min.x + destX, box.min.y, box.min.z + destZ };
        doorConfig.openMax = { box.max.x + destX, box.max.y, box.max.z + destZ };
        doorConfig.targetOpen = false;
        doorConfig.texture = texture;
        doorConfig.progress = 0.0f;

        //Add to list
        doors.push_back(doorConfig);
    }
}


void Doors::Update()
{
    //Clear out all doors and reinit the ones from the loaded map
    if (!initialized)
    {
        InitDoors();
        initialized = true;
    }

    //Toggle door state with E
    if (IsKeyPressed(KEY_E))
    {
        //Raycast setup
        Vector3 normal = Vector3Normalize({ camera.target.x - camera.position.x, camera.target.y - camera.position.y, camera.target.z - camera.position.z });
        Ray ray{ camera.position, normal };


        //Select nearest door
        float bestDist = FLT_MAX;
        int   nearestDoorIndex = -1;

        for (int i = 0; i < (int)doors.size(); i++)
        {
            //Get the collision
            auto rayHit = GetRayCollisionBox(ray, *doors[i].box);

            //If we hit a door and its the one were directly looking at, get its index and toggle the door at the index open/closed
            if (rayHit.hit && rayHit.distance < bestDist)
            {
                bestDist = rayHit.distance;
                nearestDoorIndex = i;
            }
        }


        //Toogle door at the index of the door we hit
        if (nearestDoorIndex >= 0) doors[nearestDoorIndex].targetOpen = !doors[nearestDoorIndex].targetOpen;
    }



    //Animate every door
    float speed = 2.0f;//How fast to open/close
    float deltaTime = GetFrameTime();


    for (auto& door : doors)
    {
        //Move progress toward targetOpen
        float direction = door.targetOpen ? +1.0f : -1.0f;
        door.progress = Clamp(door.progress + direction * speed * deltaTime, 0.0f, 1.0f);

        //Lerp bounding box
        door.box->min = Vector3Lerp(door.closedMin, door.openMin, door.progress);
        door.box->max = Vector3Lerp(door.closedMax, door.openMax, door.progress);
    }
}


void Doors::Draw()
{
    for (auto& door : doors)
    {
        //Center + half size
        Vector3 center =
        {
            0.5f * (door.box->min.x + door.box->max.x),
            0.5f * (door.box->min.y + door.box->max.y),
            0.5f * (door.box->min.z + door.box->max.z)
        };
        float sizeX = door.box->max.x - door.box->min.x;
        float sizeY = door.box->max.y - door.box->min.y;
        float sizeZ = door.box->max.z - door.box->min.z;

        //Draw the door with 'its' stored texture
        globals.DrawCubeTexture(door.texture, center, sizeX, sizeY, sizeZ, WHITE);
    }
}