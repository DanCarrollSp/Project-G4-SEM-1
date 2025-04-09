#pragma once

#include "raylib.h"
#include "ParticleSystem.h"
#include <string>
#include <iostream>
#include <cstdio>

class ParticleEngine 
{
public:
    ParticleEngine();
    ~ParticleEngine();

    //initialization done once
    void Init();
	bool done = false;

    //Updates UI elements and adjusts current particle params
    void Update(float deltaTime);

    //Draws both the 3D preview and the 2D UI
    void Draw(Camera3D &camera);
    Shader alphaShader;

    //Retrieves the current particle parameters for particle creating
    ParticleParams GetCurrentParams() const;

private:

    //Current parameters for particle effects
    ParticleParams currentParams;
    ParticleSystem particleSystem;

    //Slider variables for all ParticleParams:
    // Spawn count
    float sliderSpawnCount;
    // Position vector sliders
    float sliderPositionX;
    float sliderPositionY;
    float sliderPositionZ;
    // Speed parameters
    float sliderMinSpeed;
    float sliderMaxSpeed;
    // Angle parameters
    float sliderMinAngle;
    float sliderMaxAngle;
    // Lifetime parameters
    float sliderMinLifetime;
    float sliderMaxLifetime;
    // Gravity vector
    float sliderGravityX;
    float sliderGravityY;
    float sliderGravityZ;
    // Start color (RGB)
    float sliderStartColorR;
    float sliderStartColorG;
    float sliderStartColorB;
    // End color (RGB)
    float sliderEndColorR;
    float sliderEndColorG;
    float sliderEndColorB;
    // Size parameters
    float sliderStartSize;
    float sliderEndSize;
    // Boolean toggles (stored as 0.0 for false, 1.0 for true)
    float sliderEnableRandomRotation;
    float sliderIsContinuous;
    float sliderFadeAlpha;
    float sliderIsEject;

    // Emission rate
    float sliderEmissionRate;
    // Ejection parameters
    float sliderEjectDirX;
    float sliderEjectDirY;
    float sliderEjectDirZ;
    float sliderEjectAngleSpread;
    //
    float sliderBackgroundBlack;

    //Helper  funct to format the current particle parameters as a string
    std::string FormatParams() const;
};
