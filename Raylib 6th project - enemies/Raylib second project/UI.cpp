#include "UI.h"

UI::UI() {
    screenWidth = GetScreenWidth();
    screenHeight = GetScreenHeight();
}

UI::~UI() {
    UnloadAssets();
}

void UI::LoadAssets(const std::string& fontPath, const std::string& fontSmallPath, const std::string& akImagePath) {
    mainFont = LoadFont(fontPath.c_str());
    smallFont = LoadFont(fontSmallPath.c_str());
    akTexture = LoadTexture(akImagePath.c_str());

	// Load the chain texture
	chainTexture = LoadTexture("resources/Chain.png");

	// Load the pause texture
	pauseTexture = LoadTexture("resources/paused.png");
}

void UI::UnloadAssets() {
    UnloadFont(mainFont);
    UnloadFont(smallFont);
    UnloadTexture(akTexture);
}

void UI::Draw(int health, int currentAmmo, int maxAmmo) {
    // --- HEALTH (top left) with outline ---
    std::string healthText = std::to_string(health) + "%";
    Vector2 healthPos = { 50, 100 };
    float fontSize = 200;
    float spacing = 2;

    // Outline pass
    for (int dx = -2; dx <= 2; dx++) {
        for (int dy = -2; dy <= 2; dy++) {
            if (dx != 0 || dy != 0) {
                DrawTextEx(mainFont, healthText.c_str(),
                    { healthPos.x + dx, healthPos.y + dy },
                    fontSize, spacing, BLACK);
            }
        }
    }

    // Main text
    DrawTextEx(mainFont, healthText.c_str(), healthPos, fontSize, spacing, RED);

    // --- AMMO (bottom left) with outline ---
    std::string ammoText = std::to_string(currentAmmo) + "/" + std::to_string(maxAmmo);
    Vector2 ammoPos = { 25, 980 };
    float ammoFontSize = 80;

    for (int dx = -2; dx <= 2; dx++) {
        for (int dy = -2; dy <= 2; dy++) {
            if (dx != 0 || dy != 0) {
                DrawTextEx(smallFont, ammoText.c_str(),
                    { ammoPos.x + dx, ammoPos.y + dy },
                    ammoFontSize, spacing, BLACK);
            }
        }
    }

    DrawTextEx(smallFont, ammoText.c_str(), ammoPos, ammoFontSize, spacing, LIGHTGRAY);

    // --- Weapon Silhouette ---
    DrawTexture(akTexture, 20, screenHeight - akTexture.height - 40, WHITE);
}

void UI::DrawStatic()
{
    DrawTexture(chainTexture, 0, 0, WHITE);
}

