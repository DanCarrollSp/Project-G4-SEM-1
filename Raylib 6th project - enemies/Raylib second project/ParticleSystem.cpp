#include "ParticleSystem.h"



//Constructor for a single particle, initializing all of its properties
Particle::Particle(
    const Vector3& pos,
    const Vector3& vel,
    const Color& col,
    float life,
    float sSize,
    float eSize) :

    position(pos),
    velocity(vel),
    color(col),
    lifetime(life),
    age(0.0f),//Start new particle at age 0
    startSize(sSize),
    endSize(eSize),
    active(true)//Particle starts active
{};


//Constructor for a particle effect system
ParticleEffect::ParticleEffect(const ParticleParams& p) : params(p)
{
    //If the effect is not continuous, spawn the initial batch immediately
    if (!params.isContinuous) SpawnParticles(params.spawnCount);
}


//Updates all particles in the system
void ParticleEffect::Update(float deltaTime)
{
    //If the effect IS continuous, emit particles over time
    if (params.isContinuous)
    {
        accumulator += deltaTime;
        float emissionInterval = 1.0f / params.emissionRate;//Interval between emitted particles

        while (accumulator >= emissionInterval)
        {
            accumulator -= emissionInterval;
            SpawnParticles(1);//Spawns a single particle each step
        }
    }


    //Update all active particles
    for (Particle& p : particles)
    {
        if (!p.active) continue;//Skips inactive particles

        p.age += deltaTime;//Increases particle age
        if (p.age >= p.lifetime)
        {
            p.active = false;//Deactivates expired particles
            continue;
        }

        //Updates particle position based on velocity
        p.position.x += p.velocity.x * deltaTime;
        p.position.y += p.velocity.y * deltaTime;
        p.position.z += p.velocity.z * deltaTime;

        //Applies gravity
        p.velocity.x += params.gravity.x * deltaTime;
        p.velocity.y += params.gravity.y * deltaTime;
        p.velocity.z += params.gravity.z * deltaTime;

        //Interpolates color over time
        float currentAge = p.age / p.lifetime;//Current progress in life cycle
        p.color = LerpColor(params.startColor, params.endColor, currentAge);

        //(Optionally) fades alpha over time
        if (params.fadeAlpha) p.color.a = (unsigned char)Lerp((float)params.startColor.a, (float)params.endColor.a, currentAge);
    }
}


//Draws all active particles
void ParticleEffect::Draw(Camera3D camera)
{
    for (const Particle& p : particles)
    {
        if (!p.active) continue;//Skips inactive particles

        //Determines the current size based on lifetime progress
        float current = p.age / p.lifetime;
        float size = Lerp(p.startSize, p.endSize, current);


        //Draws a textured particle (if texture provided else draw a colored circle)
        if (params.texture && params.texture->id != 0)
        {
            Rectangle source = { 0, 0, (float)params.texture->width, (float)params.texture->height };
            Vector2 origin = { (float)params.texture->width * 0.5f, (float)params.texture->height * 0.5f };
            float scale = size / (float)params.texture->width;


            /*DrawTexturePro(*params.texture, source,

                Rectangle{ p.position.x, p.position.y, (float)params.texture->width * scale, (float)params.texture->height * scale},

                origin, 0.0f, p.color );*/

            DrawBillboard(camera, *params.texture, p.position, size, p.color);
        }
        else
        {
            //Draws a circle if no texture available
            DrawSphere(p.position, size * 0.5f, p.color);
        }
    }
}



//Checks if the particle effect is still active
bool ParticleEffect::IsAlive() const
{
    //Continuous effects never die unless stopped manually so no need to check
    if (params.isContinuous) return true;

	//Loops through all particles in the effect, if theres an active one, the effect is still alive
    for (const Particle& p : particles) if (p.active) return true;

	//If no active particles found, the effect is finished
    return false;
}



//Spawns the particles at the emitter pos
void ParticleEffect::SpawnParticles(int count)
{
    for (int i = 0; i < count; i++)
    {
        //Generates random angles for the particles directions
		float turnDeg = (float)GetRandomValue(0, 360);//Left to right angle
        float turnRad = turnDeg * (PI / 180.0f);

		float tiltDeg = (float)GetRandomValue((int)params.minAngle, (int)params.maxAngle);//Up to down angle
        float tiltRad = tiltDeg * (PI / 180.0f);

        //Generates a random speed for the particle
        float speed = (float)GetRandomValue((int)params.minSpeed, (int)params.maxSpeed);

        // Convert spherical coordinates to Cartesian for movement direction
        Vector3 vel = {
            cosf(turnRad) * sinf(tiltRad) * speed, ///X turn
            cosf(tiltRad) * speed,                 //Y tilt
            sinf(turnRad) * sinf(tiltRad) * speed  //Z turn
        };

        //Assigns a random lifespan
        float life = GetRandomFloat(params.minLifetime, params.maxLifetime);

        //Creates and stores this new particle
        Particle p(params.position, vel, params.startColor, life, params.startSize, params.endSize);
        particles.push_back(p);
    }
}

//Linear interpolation for float values
float ParticleEffect::Lerp(float start, float end, float current)
{
    return start + (end - start) * current;
}

//Linear interpolation for colors
Color ParticleEffect::LerpColor(const Color& start, const Color& end, float current)
{
    Color result;
    result.r = (unsigned char)Lerp((float)start.r, (float)end.r, current);
    result.g = (unsigned char)Lerp((float)start.g, (float)end.g, current);
    result.b = (unsigned char)Lerp((float)start.b, (float)end.b, current);
    result.a = (unsigned char)Lerp((float)start.a, (float)end.a, current);
    return result;
}

//Generates a random float within a given range
float ParticleEffect::GetRandomFloat(float min, float max)
{
    float randomF = (float)rand() / (float)RAND_MAX;
    return min + randomF * (max - min);
}



//Creates a new particle effect and stores it
void ParticleSystem::Instantiate(const ParticleParams& params)
{
    effects.push_back(std::make_unique<ParticleEffect>(params));
}



//Updates all particle effects and removes finished ones
void ParticleSystem::UpdateAll(float deltaTime)
{
    for (auto it = effects.begin(); it != effects.end();)
    {
		//Updates the effect (updating active particles who've outlived their lifespan will deactivate them)
        (*it)->Update(deltaTime);

		//If an effect is deactivated, erase it from storage, else move to the next effect
        if (!(*it)->IsAlive()) it = effects.erase(it);
        else ++it;
    }
}

//Draws all active particle effects
void ParticleSystem::DrawAll(Camera3D camera)
{
    for (auto& effect : effects)  effect->Draw(camera);
}
