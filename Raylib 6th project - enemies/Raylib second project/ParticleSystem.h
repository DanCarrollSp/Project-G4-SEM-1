#pragma once

#include <raylib.h>
#include <vector>
#include <memory>



//Struct to define the parameters that control the behavior of a particle effect
struct ParticleParams
{
    int     spawnCount = 20;                 //Number of particles to spawn initially
    Vector3 position = { 0.0f, 0.5f, 0.0f }; //Initial spawn position of particles
    float   minSpeed = 50.0f;                //Minimum speed of a particle
    float   maxSpeed = 150.0f;               //Maximum speed of a particle
    float   minAngle = 0.0f;                 //Minimum angle for emission direction
    float   maxAngle = 360.0f;               //Maximum angle for emission direction
    float   minLifetime = 1.0f;              //Minimum lifetime of a particle
    float   maxLifetime = 2.0f;              //Maximum lifetime of a particle
    Vector3 gravity = { 0.0f, 0.0f, 0.0f };  //Gravity applied to particles
    Color   startColor = PINK;               //Initial color of the particle
    Color   endColor = DARKGRAY;             //Final color of the particle
    float   startSize = 8.0f;                //Initial size of the particle
    float   endSize = 2.0f;                  //Final size of the particle
	bool    isContinuous = false;            //Continuously spawns particles if true or a burst if false
    float   emissionRate = 10.0f;            //Number of particles emitted per second (if continuous)
    Texture2D* texture = nullptr;            //Pointer to an OPTIONAL texture for the particles
    bool    fadeAlpha = true;                //Whether the particles alpha should fade over time
};



//Struct for individual particles in the system
struct Particle
{
    Vector3 position;   //Current position of the particle
    Vector3 velocity;   //Movement direction and speed
    Color   color;      //Current color of the particle
    float   lifetime;   //Total duration before particle disappears
    float   age;        //Current age of the particle (time alive)
    float   startSize;  //Initial size
    float   endSize;    //Final size
    bool    active;     //If its currently active

    //Constructor to initialize this individual particle
    Particle(const Vector3& pos, const Vector3& vel, const Color& col, float life, float sSize, float eSize);
};



//Class to manage the group of particles and updates/draws them
class ParticleEffect
{
public:

	//Constructor: Initializes a particle effect with given parameters (explicit stops autocoverting of ParticleParams object into a ParticleEffect object)
    explicit ParticleEffect(const ParticleParams& p);

    
    void Update(float deltaTime);   //Updates particles, Handling movement, gravity, and lifespan decay
    void Draw(Camera3D camera);     //Renders all active particles
    bool IsAlive() const;           //Returns if the effect is still “alive” (has active particles and should be drawn)


private:

    //Spawns X new particles at once
    void SpawnParticles(int count);
    
    
    static float Lerp(float start, float end, float current);                       //Linear interpolation for float values
    static Color LerpColor(const Color& start, const Color& end, float current);    //Linear interpolation for colors (smoother color transitions)

    static float GetRandomFloat(float min, float max);//Generates a random float value within a range



    ParticleParams params;           //Parameters for the effect
    std::vector<Particle> particles; //List of all the individual particles in the effect
    float accumulator = 0.0f;        //Keeps track of time for continuous emission
};



//This class maintains multiple active particle effects
class ParticleSystem
{
public:

    void Instantiate(const ParticleParams& params);//Creates a new ParticleEffect instance and stores it
    void UpdateAll(float deltaTime);//Updates all active particle effects and removes finished ones
    void DrawAll(Camera3D camera);//Draws all active particle effects in the scene


private:

    std::vector<std::unique_ptr<ParticleEffect>> effects;//Stores all active particle effects
};
