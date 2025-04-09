// main.cpp
#include <raylib.h>


#include "raygui.h"

#include <vector>
#include <string>
#include <cmath>

// Utility linear interpolation function.
float Lerp(float start, float end, float t) {
    return start + (end - start) * t;
}

//---------------------------------------------------------------------------
// Particle system structures and classes
//---------------------------------------------------------------------------

// Structure to hold parameters that control a particle effect
struct ParticleParams {
    float spawnCount = 20;
    Vector3 position = { 0.0f, 0.0f, 0.0f };
    float minSpeed = 5.0f;
    float maxSpeed = 20.0f;
    float minAngle = 0.0f;
    float maxAngle = 180.0f;
    float minLifetime = 1.0f;
    float maxLifetime = 2.0f;
    Vector3 gravity = { 0.0f, -9.8f, 0.0f };
    Color startColor = RED;
    Color endColor = BLUE;
    float startSize = 0.5f;
    float endSize = 0.1f;
    bool enableRandomRotation = false;
    bool isContinuous = false;
    float emissionRate = 10.0f;
    bool fadeAlpha = true;
};

// Structure that defines an individual particle
struct Particle {
    Vector3 position;
    Vector3 velocity;
    Color color;
    float lifetime;
    float age;
    float startSize;
    float endSize;
    float rotation;
    bool active;
};

// A simple ParticleEffect that spawns, updates, and draws particles
struct ParticleEffect {
    ParticleParams params;
    std::vector<Particle> particles;
    float accumulator = 0.0f;

    // Spawns a single particle using the current parameters
    void SpawnParticle() {
        // Generate a random angle (converted to radians)
        float angleDeg = (float)GetRandomValue((int)params.minAngle, (int)params.maxAngle);
        float angle = angleDeg * DEG2RAD;
        // Generate a speed within range
        float speed = (float)GetRandomValue((int)params.minSpeed, (int)params.maxSpeed);
        // Create a simple 2D directional vector (extended to 3D: zero Z)
        Vector3 dir = { cosf(angle) * speed, sinf(angle) * speed, 0.0f };

        // Determine a random lifetime
        float lifetime = params.minLifetime + (GetRandomValue(0, 1000) / 1000.0f) * (params.maxLifetime - params.minLifetime);
        // Push the new particle
        Particle p = { params.position, dir, params.startColor, lifetime, 0.0f, params.startSize, params.endSize,
                       params.enableRandomRotation ? (float)GetRandomValue(0, 360) : 0.0f, true };
        particles.push_back(p);
    }

    // Update particles based on dt (delta time)
    void Update(float dt) {
        if (params.isContinuous) {
            accumulator += dt;
            float interval = 1.0f / params.emissionRate;
            while (accumulator >= interval) {
                accumulator -= interval;
                SpawnParticle();
            }
        }

        for (auto& p : particles) {
            if (!p.active) continue;

            p.age += dt;
            if (p.age >= p.lifetime) {
                p.active = false;
                continue;
            }

            float t = p.age / p.lifetime;
            // Update the particle's position
            p.position.x += p.velocity.x * dt;
            p.position.y += p.velocity.y * dt;
            p.position.z += p.velocity.z * dt;

            // Apply gravity
            p.velocity.x += params.gravity.x * dt;
            p.velocity.y += params.gravity.y * dt;
            p.velocity.z += params.gravity.z * dt;

            // Update color (linearly interpolate between start and end colors)
            p.color.r = (unsigned char)Lerp((float)params.startColor.r, (float)params.endColor.r, t);
            p.color.g = (unsigned char)Lerp((float)params.startColor.g, (float)params.endColor.g, t);
            p.color.b = (unsigned char)Lerp((float)params.startColor.b, (float)params.endColor.b, t);
            p.color.a = (unsigned char)Lerp((float)params.startColor.a, (float)params.endColor.a, t);

            if (params.fadeAlpha)
                p.color.a = (unsigned char)((1.0f - t) * 255.0f);
        }
    }

    // Draw each active particle as a sphere
    void Draw() {
        for (auto& p : particles) {
            if (!p.active) continue;
            float t = p.age / p.lifetime;
            float size = Lerp(p.startSize, p.endSize, t);
            DrawSphere(p.position, size, p.color);
        }
    }

    // Resets the particle effect (clears particles and spawns a burst if not continuous)
    void Reset() {
        particles.clear();
        accumulator = 0.0f;
        if (!params.isContinuous) {
            for (int i = 0; i < params.spawnCount; i++) {
                SpawnParticle();
            }
        }
    }
};

//---------------------------------------------------------------------------
// GUI Functionality (using raygui)
//---------------------------------------------------------------------------

void DrawUI(ParticleParams& p, ParticleEffect& effect) {
    float x = 20, y = 20, spacing = 30;

    p.spawnCount = GuiSliderBar({ x, y, 200, 20 },
        "Spawn Count", TextFormat("%d", p.spawnCount),
        &p.spawnCount, 1, 100);
    y += spacing;
    p.minSpeed = GuiSliderBar({ (float)x, (float)y, 200, 20 },
        "Min Speed", TextFormat("%.1f", p.minSpeed),
        &p.minSpeed, 0, p.maxSpeed);
    y += spacing;
    p.maxSpeed = GuiSliderBar({ (float)x, (float)y, 200, 20 },
        "Max Speed", TextFormat("%.1f", p.maxSpeed),
        &p.maxSpeed, p.minSpeed, 100);
    y += spacing;
    p.minLifetime = GuiSliderBar({ (float)x, (float)y, 200, 20 },
        "Min Lifetime", TextFormat("%.1f", p.minLifetime),
        &p.minLifetime, 0.1f, p.maxLifetime);
    y += spacing;
    p.maxLifetime = GuiSliderBar({ (float)x, (float)y, 200, 20 },
        "Max Lifetime", TextFormat("%.1f", p.maxLifetime),
        &p.maxLifetime, p.minLifetime, 10.0f);
    y += spacing;
    p.startSize = GuiSliderBar({ (float)x, (float)y, 200, 20 },
        "Start Size", TextFormat("%.2f", p.startSize),
        &p.startSize, 0.1f, 2.0f);
    y += spacing;
    p.endSize = GuiSliderBar({ (float)x, (float)y, 200, 20 },
        "End Size", TextFormat("%.2f", p.endSize),
        &p.endSize, 0.1f, 2.0f);
    y += spacing;
    p.gravity.y = GuiSliderBar({ (float)x, (float)y, 200, 20 },
        "Gravity Y", TextFormat("%.1f", p.gravity.y),
        &p.gravity.y, -20, 20);
    y += spacing;
    p.isContinuous = GuiCheckBox({ (float)x, (float)y, 20, 20 }, "Continuous", &p.isContinuous);
    y += spacing;
    p.fadeAlpha = GuiCheckBox({ (float)x, (float)y, 20, 20 }, "Fade Alpha", &p.fadeAlpha);
    y += spacing;

    // "Copy Params" button: copies current settings as C++ code to the clipboard
    if (GuiButton({ (float)x, (float)y, 120, 30 }, "COPY PARAMS")) {
        std::string str = TextFormat(
            "ParticleParams p = {\n"
            "    %d,\n"
            "    { %.2ff, %.2ff, %.2ff },\n"
            "    %.2ff, %.2ff,\n"
            "    %.2ff, %.2ff,\n"
            "    %.2ff, %.2ff,\n"
            "    { %.2ff, %.2ff, %.2ff },\n"
            "    { %d, %d, %d, %d },\n"
            "    { %d, %d, %d, %d },\n"
            "    %.2ff, %.2ff,\n"
            "    %s,\n"
            "    %s,\n"
            "    %.2ff,\n"
            "    nullptr,\n"
            "    %s\n"
            "};",
            p.spawnCount,
            p.position.x, p.position.y, p.position.z,
            p.minSpeed, p.maxSpeed,
            p.minAngle, p.maxAngle,
            p.minLifetime, p.maxLifetime,
            p.gravity.x, p.gravity.y, p.gravity.z,
            p.startColor.r, p.startColor.g, p.startColor.b, p.startColor.a,
            p.endColor.r, p.endColor.g, p.endColor.b, p.endColor.a,
            p.startSize, p.endSize,
            p.enableRandomRotation ? "true" : "false",
            p.isContinuous ? "true" : "false",
            p.emissionRate,
            p.fadeAlpha ? "true" : "false"
        );
        SetClipboardText(str.c_str());
    }

    // "Reset" button to clear and respawn particles based on current settings
    if (GuiButton({ (float)x + 130, (float)y, 80, 30 }, "Reset")) {
        effect.Reset();
    }
}

//---------------------------------------------------------------------------
// Main entry point
//---------------------------------------------------------------------------

int main(void) {
    InitWindow(1280, 720, "Particle Builder - Minimal Viewer (Safe Versions)");
    SetTargetFPS(60);

    // Set up a simple 3D camera
    Camera3D camera = { 0 };
    camera.position = { 0.0f, 5.0f, 15.0f };
    camera.target = { 0.0f, 2.0f, 0.0f };
    camera.up = { 0.0f, 1.0f, 0.0f };
    camera.fovy = 60.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    // Create a default ParticleParams instance and a ParticleEffect
    ParticleParams params;
    ParticleEffect effect;
    effect.params = params;
    effect.Reset();

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();

        // Update the effect's parameters and then update particles
        effect.params = params;
        effect.Update(dt);

        BeginDrawing();
        ClearBackground(BLACK);

        // 3D scene
        BeginMode3D(camera);
        DrawGrid(10, 1.0f);
        effect.Draw();
        EndMode3D();

        // Draw GUI over the scene
        DrawUI(params, effect);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
