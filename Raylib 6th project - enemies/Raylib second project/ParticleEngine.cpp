#include "ParticleEngine.h"
#include <raymath.h>
#include <string>

//Global preview timer (for the 3D preview burst)
float previewTimer = 0.0f;

//Pause bool
static bool paused = false;

//Helper function to draw and update a sliders
//Draws a composed string (label and value) centered inside the slider box
static float UpdateSlider(const char* label, float value, float min, float max, float posX, float posY, float sliderWidth, float sliderHeight, float knobRadius)
{
    Rectangle bar = { posX, posY, sliderWidth, sliderHeight };
    Vector2 mouse = GetMousePosition();

    //Updates slider value if the mouse is pressed in the area
    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(mouse, bar))
    {
        value = ((mouse.x - posX) / sliderWidth) * (max - min) + min;
        value = Clamp(value, min, max);
    }

    //Draws the slider bar
    DrawRectangleRec(bar, LIGHTGRAY);

    //Draw the knob/handdle on the slider
    float normalized = (value - min) / (max - min);
    float knobX = posX + normalized * sliderWidth;
    DrawCircle((int)knobX, (int)(posY + sliderHeight / 2), knobRadius, DARKGRAY);

    //Centers the text in the slider box
    int fontSize = 10;
    std::string displayStr = std::string(label) + ": " + TextFormat("%.2f", value);
    int textWidth = MeasureText(displayStr.c_str(), fontSize);
    int textX = posX + (sliderWidth - textWidth) / 2;
    int textY = posY + (sliderHeight - fontSize) / 2;
    DrawText(displayStr.c_str(), textX, textY, fontSize, BLACK);

    return value;
}

//Helper function to draw and update checkboxes
//Draws a square and, if checked, fills it, the name/label is drawn on the right of it
static bool UpdateCheckbox(const char* label, bool checked, float posX, float posY, float boxSize)
{
    Rectangle box = { posX, posY, boxSize, boxSize };
    Vector2 mouse = GetMousePosition();

    //Toggles the checkbox if clicked
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(mouse, box))
    {
        checked = !checked;
    }

    //Draws the checkbox background
    DrawRectangleRec(box, LIGHTGRAY);
    //If checked, draw a filled rectangle
    if (checked)
    {
        DrawRectangleRec(box, DARKGRAY);
    }

    //Draws what checkbox it is next to the checkbox
    int fontSize = 10;
    DrawText(label, posX + boxSize + 5, posY + (int)((boxSize - fontSize) / 2), fontSize, BLACK);

    return checked;
}

ParticleEngine::ParticleEngine() 
{
    //Default particle params
    currentParams.spawnCount = 20;
    currentParams.position = { 0.0f, 0.5f, 0.0f };
    currentParams.minSpeed = 50.0f;
    currentParams.maxSpeed = 150.0f;
    currentParams.minAngle = 0.0f;
    currentParams.maxAngle = 360.0f;
    currentParams.minLifetime = 1.0f;
    currentParams.maxLifetime = 2.0f;
    currentParams.gravity = { 0.0f, 0.0f, 0.0f };
    currentParams.startColor = PINK;
    currentParams.endColor = DARKGRAY;
    currentParams.startSize = 8.0f;
    currentParams.endSize = 2.0f;
    currentParams.enableRandomRotation = false;
    currentParams.isContinuous = false;
    currentParams.emissionRate = 10.0f;
    currentParams.fadeAlpha = true;
    currentParams.isEject = false;
    currentParams.ejectDirection = { 0.0f, 1.0f, 0.0f };
    currentParams.ejectAngleSpread = 30.0f;
    currentParams.texture = nullptr;  //No texture by default

    //Initialize slider variables with default values
    sliderSpawnCount = 20.0f;
    sliderPositionX = 0.0f;
    sliderPositionY = 0.5f;
    sliderPositionZ = 0.0f;
    sliderMinSpeed = currentParams.minSpeed;
    sliderMaxSpeed = currentParams.maxSpeed;
    sliderMinAngle = currentParams.minAngle;
    sliderMaxAngle = currentParams.maxAngle;
    sliderMinLifetime = currentParams.minLifetime;
    sliderMaxLifetime = currentParams.maxLifetime;
    sliderGravityX = currentParams.gravity.x;
    sliderGravityY = currentParams.gravity.y;
    sliderGravityZ = currentParams.gravity.z;
    sliderStartColorR = PINK.r; sliderStartColorG = PINK.g; sliderStartColorB = PINK.b;
    sliderEndColorR = DARKGRAY.r; sliderEndColorG = DARKGRAY.g; sliderEndColorB = DARKGRAY.b;
    sliderStartSize = currentParams.startSize;
    sliderEndSize = currentParams.endSize;
    //For bools, store 0.0 for false and 1.0 as true
    sliderEnableRandomRotation = 0.0f;
    sliderIsContinuous = 0.0f;
    sliderEmissionRate = currentParams.emissionRate;
    sliderFadeAlpha = 1.0f;
    sliderIsEject = 0.0f;
    sliderEjectDirX = currentParams.ejectDirection.x;
    sliderEjectDirY = currentParams.ejectDirection.y;
    sliderEjectDirZ = currentParams.ejectDirection.z;
    sliderEjectAngleSpread = currentParams.ejectAngleSpread;
    //
    sliderBackgroundBlack = 0.0f;
}

ParticleEngine::~ParticleEngine() 
{
    //Frees the texture if one was loaded
    if (currentParams.texture != nullptr) {
        UnloadTexture(*currentParams.texture);
        delete currentParams.texture;
        currentParams.texture = nullptr;
    }
}

void ParticleEngine::Init() 
{
    //Only done once here

	//Load the shader for alpha blending
    alphaShader = LoadShader(NULL, "shaders/alpha.fs");
}


// - Update -
//Particle system instantiation,
//handles dropped texture files,
//Allows for pausing and unpausing,
//Updates params based on sliders and checkboxes
void ParticleEngine::Update(float deltaTime) {

    if (!done)
    {
        Init();
        done = true;
    }

    //Toggle pause if Space is pressed
    if (IsKeyPressed(KEY_SPACE)) paused = !paused;

    //Check for dropped files
    if (IsFileDropped()) {
        FilePathList droppedFiles = LoadDroppedFiles();
        if (droppedFiles.count > 0) {
            Texture2D loadedTex = LoadTexture(droppedFiles.paths[0]);
            if (currentParams.texture != nullptr) {
                UnloadTexture(*currentParams.texture);
                delete currentParams.texture;
            }
            currentParams.texture = new Texture2D();
            *currentParams.texture = loadedTex;
        }
        UnloadDroppedFiles(droppedFiles);
    }


    //Updates current particle parameters from slider values
    currentParams.spawnCount = static_cast<int>(sliderSpawnCount);
    currentParams.position = { sliderPositionX, sliderPositionY, sliderPositionZ };
    currentParams.minSpeed = sliderMinSpeed;
    currentParams.maxSpeed = sliderMaxSpeed;
    currentParams.minAngle = sliderMinAngle;
    currentParams.maxAngle = sliderMaxAngle;
    currentParams.minLifetime = sliderMinLifetime;
    currentParams.maxLifetime = sliderMaxLifetime;
    currentParams.gravity = { sliderGravityX, sliderGravityY, sliderGravityZ };
    currentParams.startColor = { (unsigned char)sliderStartColorR, (unsigned char)sliderStartColorG, (unsigned char)sliderStartColorB, 255 };
    currentParams.endColor = { (unsigned char)sliderEndColorR, (unsigned char)sliderEndColorG, (unsigned char)sliderEndColorB, 255 };
    currentParams.startSize = sliderStartSize;
    currentParams.endSize = sliderEndSize;
    currentParams.enableRandomRotation = (sliderEnableRandomRotation > 0.5f);
    currentParams.isContinuous = (sliderIsContinuous > 0.5f);
    currentParams.emissionRate = sliderEmissionRate;
    currentParams.fadeAlpha = (sliderFadeAlpha > 0.5f);
    currentParams.isEject = (sliderIsEject > 0.5f);
    currentParams.ejectDirection = { sliderEjectDirX, sliderEjectDirY, sliderEjectDirZ };
    currentParams.ejectAngleSpread = sliderEjectAngleSpread;

    //If the sim is not paused, update the preview bursts
    if (!paused) {
        previewTimer += GetFrameTime();
        if (previewTimer > 1.0f)
        {
            ParticleParams previewParams = currentParams;
            previewParams.position = { 0, 1, 0 };
            previewParams.isContinuous = false;
            particleSystem.Instantiate(previewParams);
            previewTimer = 0.0f;
        }
    }
}



void ParticleEngine::Draw(Camera3D& camera) 
{
	//Can move the camera when right mouse button is held
    if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) UpdateCamera(&camera, CAMERA_FREE);

    //Use black background if sliderBackgroundBlack > 0.5, else white
    bool useBlackBG = (sliderBackgroundBlack > 0.5f);



    /// //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    BeginDrawing();
    ClearBackground(useBlackBG ? BLACK : WHITE);
    BeginShaderMode(alphaShader);


    /// ///////////////////////////////////////////////////////////
    BeginMode3D(camera);

    DrawGrid(10, 1.0f);
    // Only update particles if not paused.
    if (!paused) particleSystem.UpdateAll(GetFrameTime());
    particleSystem.DrawAll(camera);

    EndShaderMode();
    EndMode3D();

    DrawText("Hold Right Mouse Button to enable movement (WASD, Space, Ctrl)", 350, 50, 20, LIGHTGRAY);
    if (paused) DrawText("PAUSED", 350, 70, 20, RED);

    //Sliders, Checkboxes and Buttons
    float uiX = 20.0f;
    float uiY = 20.0f;
    float sliderWidth = 300.0f;
    float sliderHeight = 20.0f;
    float spacing = 35.0f;
    float knobRadius = 10.0f;

    //Particle params
    sliderSpawnCount = UpdateSlider("Spawn Count", sliderSpawnCount, 1.0f, 101.0f, uiX, uiY, sliderWidth, sliderHeight, knobRadius); uiY += spacing;
    sliderMinSpeed = UpdateSlider("Min Speed", sliderMinSpeed, 0.0f, 200.0f, uiX, uiY, sliderWidth, sliderHeight, knobRadius); uiY += spacing;
    sliderMaxSpeed = UpdateSlider("Max Speed", sliderMaxSpeed, sliderMinSpeed, 300.0f, uiX, uiY, sliderWidth, sliderHeight, knobRadius); uiY += spacing;
    sliderMinAngle = UpdateSlider("Min Angle", sliderMinAngle, 0.0f, 360.0f, uiX, uiY, sliderWidth, sliderHeight, knobRadius); uiY += spacing;
    sliderMaxAngle = UpdateSlider("Max Angle", sliderMaxAngle, sliderMinAngle, 360.0f, uiX, uiY, sliderWidth, sliderHeight, knobRadius); uiY += spacing;
    sliderMinLifetime = UpdateSlider("Min Lifetime", sliderMinLifetime, 0.1f, 10.0f, uiX, uiY, sliderWidth, sliderHeight, knobRadius); uiY += spacing;
    sliderMaxLifetime = UpdateSlider("Max Lifetime", sliderMaxLifetime, sliderMinLifetime, 10.0f, uiX, uiY, sliderWidth, sliderHeight, knobRadius); uiY += spacing;
    sliderGravityX = UpdateSlider("Gravity X", sliderGravityX, -50.0f, 50.0f, uiX, uiY, sliderWidth, sliderHeight, knobRadius); uiY += spacing;
    sliderGravityY = UpdateSlider("Gravity Y", sliderGravityY, -50.0f, 50.0f, uiX, uiY, sliderWidth, sliderHeight, knobRadius); uiY += spacing;
    sliderGravityZ = UpdateSlider("Gravity Z", sliderGravityZ, -50.0f, 50.0f, uiX, uiY, sliderWidth, sliderHeight, knobRadius); uiY += spacing;
    sliderStartColorR = UpdateSlider("Start Color R", sliderStartColorR, 0.0f, 255.0f, uiX, uiY, sliderWidth, sliderHeight, knobRadius); uiY += spacing;
    sliderStartColorG = UpdateSlider("Start Color G", sliderStartColorG, 0.0f, 255.0f, uiX, uiY, sliderWidth, sliderHeight, knobRadius); uiY += spacing;
    sliderStartColorB = UpdateSlider("Start Color B", sliderStartColorB, 0.0f, 255.0f, uiX, uiY, sliderWidth, sliderHeight, knobRadius); uiY += spacing;
    sliderEndColorR = UpdateSlider("End Color R", sliderEndColorR, 0.0f, 255.0f, uiX, uiY, sliderWidth, sliderHeight, knobRadius); uiY += spacing;
    sliderEndColorG = UpdateSlider("End Color G", sliderEndColorG, 0.0f, 255.0f, uiX, uiY, sliderWidth, sliderHeight, knobRadius); uiY += spacing;
    sliderEndColorB = UpdateSlider("End Color B", sliderEndColorB, 0.0f, 255.0f, uiX, uiY, sliderWidth, sliderHeight, knobRadius); uiY += spacing;
    sliderStartSize = UpdateSlider("Start Size", sliderStartSize, 1.0f, 50.0f, uiX, uiY, sliderWidth, sliderHeight, knobRadius); uiY += spacing;
    sliderEndSize = UpdateSlider("End Size", sliderEndSize, 0.1f, sliderStartSize, uiX, uiY, sliderWidth, sliderHeight, knobRadius); uiY += spacing;

    //Bool toggles - checkboxes
    float checkBoxSize = 20.0f;
    {
        bool check = (sliderEnableRandomRotation > 0.5f);
        check = UpdateCheckbox("Enable Random Rotation", check, uiX, uiY, checkBoxSize);
        sliderEnableRandomRotation = check ? 1.0f : 0.0f;
        uiY += spacing;
    }
    {
        bool check = (sliderIsContinuous > 0.5f);
        check = UpdateCheckbox("Is Continuous", check, uiX, uiY, checkBoxSize);
        sliderIsContinuous = check ? 1.0f : 0.0f;
        uiY += spacing;
    }
    {
        bool check = (sliderFadeAlpha > 0.5f);
        check = UpdateCheckbox("Fade Alpha", check, uiX, uiY, checkBoxSize);
        sliderFadeAlpha = check ? 1.0f : 0.0f;
        uiY += spacing;
    }
    {
        bool check = (sliderIsEject > 0.5f);
        check = UpdateCheckbox("Is Eject", check, uiX, uiY, checkBoxSize);
        sliderIsEject = check ? 1.0f : 0.0f;
        uiY += spacing;
    }
    //
    {
        bool check = (sliderBackgroundBlack > 0.5f);
        check = UpdateCheckbox("Black Background", check, uiX, uiY, checkBoxSize);
        sliderBackgroundBlack = check ? 1.0f : 0.0f;
        uiY += spacing;
    }

    //Emission rate
    sliderEmissionRate = UpdateSlider("Emission Rate", sliderEmissionRate, 1.0f, 120.0f, uiX, uiY, sliderWidth, sliderHeight, knobRadius); uiY += spacing;
    //Eject direction vector
    sliderEjectDirX = UpdateSlider("Eject Dir X", sliderEjectDirX, -10.0f, 10.0f, uiX, uiY, sliderWidth, sliderHeight, knobRadius); uiY += spacing;
    sliderEjectDirY = UpdateSlider("Eject Dir Y", sliderEjectDirY, -10.0f, 10.0f, uiX, uiY, sliderWidth, sliderHeight, knobRadius); uiY += spacing;
    sliderEjectDirZ = UpdateSlider("Eject Dir Z", sliderEjectDirZ, -10.0f, 10.0f, uiX, uiY, sliderWidth, sliderHeight, knobRadius); uiY += spacing;
    sliderEjectAngleSpread = UpdateSlider("Eject Angle Spread", sliderEjectAngleSpread, 0.0f, 180.0f, uiX, uiY, sliderWidth, sliderHeight, knobRadius); uiY += spacing;

    // "Copy Params to Clipboard" Button
    Rectangle buttonRect = { uiX, uiY, sliderWidth, sliderHeight };
    DrawRectangleRec(buttonRect, LIGHTGRAY);
    DrawText("Copy Params to Clipboard", uiX + 5, uiY + 5, 10, DARKGRAY);
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(GetMousePosition(), buttonRect))
    {
        std::string paramsText = FormatParams();
        SetClipboardText(paramsText.c_str());
    }


    //Display texture status message
    if (currentParams.texture == nullptr) DrawText("Drop a texture file onto the window to apply to particles", 350, 80, 10, DARKGRAY);
    else DrawText("Texture loaded and applied to particles", 350, 80, 10, DARKGRAY);

    EndDrawing();
}


ParticleParams ParticleEngine::GetCurrentParams() const 
{
    return currentParams;
}


std::string ParticleEngine::FormatParams() const 
{
    char buffer[1024];
    sprintf_s(buffer,
        "ParticleParams params = {\n"
        "    spawnCount = %d,\n"
        "    position = { %.2f, %.2f, %.2f },\n"
        "    minSpeed = %.1f,\n"
        "    maxSpeed = %.1f,\n"
        "    minAngle = %.1f,\n"
        "    maxAngle = %.1f,\n"
        "    minLifetime = %.1f,\n"
        "    maxLifetime = %.1f,\n"
        "    gravity = { %.2f, %.2f, %.2f },\n"
        "    startColor = { %d, %d, %d },\n"
        "    endColor = { %d, %d, %d },\n"
        "    startSize = %.1f,\n"
        "    endSize = %.1f,\n"
        "    enableRandomRotation = %s,\n"
        "    isContinuous = %s,\n"
        "    emissionRate = %.1f,\n"
        "    fadeAlpha = %s,\n"
        "    isEject = %s,\n"
        "    ejectDirection = { %.2f, %.2f, %.2f },\n"
        "    ejectAngleSpread = %.1f\n"
        "};",
        currentParams.spawnCount,
        currentParams.position.x, currentParams.position.y, currentParams.position.z,
        currentParams.minSpeed,
        currentParams.maxSpeed,
        currentParams.minAngle,
        currentParams.maxAngle,
        currentParams.minLifetime,
        currentParams.maxLifetime,
        currentParams.gravity.x, currentParams.gravity.y, currentParams.gravity.z,
        currentParams.startColor.r, currentParams.startColor.g, currentParams.startColor.b,
        currentParams.endColor.r, currentParams.endColor.g, currentParams.endColor.b,
        currentParams.startSize,
        currentParams.endSize,
        currentParams.enableRandomRotation ? "true" : "false",
        currentParams.isContinuous ? "true" : "false",
        currentParams.emissionRate,
        currentParams.fadeAlpha ? "true" : "false",
        currentParams.isEject ? "true" : "false",
        currentParams.ejectDirection.x, currentParams.ejectDirection.y, currentParams.ejectDirection.z,
        currentParams.ejectAngleSpread
    );
    return std::string(buffer);
}