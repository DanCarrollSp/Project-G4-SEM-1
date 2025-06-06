#include "UI.h"

UI::UI() 
{
    screenWidth = GetScreenWidth();
    screenHeight = GetScreenHeight();
}

UI::~UI() 
{
    UnloadAssets();
}

void UI::LoadAssets(const std::string& fontPath, const std::string& fontSmallPath, const std::string& akImagePath) 
{
    mainFont = LoadFont(fontPath.c_str());
    smallFont = LoadFont(fontSmallPath.c_str());
    akTexture = LoadTexture(akImagePath.c_str());

	//Load the chain texture
	chainTexture = LoadTexture("resources/Chain.png");

	//Load the pause texture
	pauseTexture = LoadTexture("resources/paused.png");
}

void UI::UnloadAssets() 
{
    UnloadFont(mainFont);
    UnloadFont(smallFont);
    UnloadTexture(akTexture);
}

void UI::Draw(int health, int currentAmmo, int maxAmmo) 
{
    //Health top left - with outline
    std::string healthText = std::to_string(health) + "%";
    Vector2 healthPos = { 50, 100 };
    float fontSize = 200;
    float spacing = 2;

	//Outline (remake the health twice, slightly offset and black)
    for (int dx = -2; dx <= 2; dx++) 
    {
		//Derivative of the outline
        for (int dy = -2; dy <= 2; dy++) 
        {
            if (dx != 0 || dy != 0)
            {
                DrawTextEx(mainFont, healthText.c_str(), { healthPos.x + dx, healthPos.y + dy }, fontSize, spacing, BLACK);
            }
        }
    }
    //Main health text (under outline so its drawn on top)
    DrawTextEx(mainFont, healthText.c_str(), healthPos, fontSize, spacing, RED);



    //AMMO (bottom left) with same outline
    std::string ammoText = std::to_string(currentAmmo) + "/" + std::to_string(maxAmmo);
    Vector2 ammoPos = { 25, 980 };
    float ammoFontSize = 80;

    //Outline (remake the ammo twice, slightly offset on either side and black)
    for (int dx = -2; dx <= 2; dx++)
    {
        //Derivative of the outline
        for (int dy = -2; dy <= 2; dy++)
        {
            if (dx != 0 || dy != 0)
            {
                DrawTextEx(smallFont, ammoText.c_str(), { ammoPos.x + dx, ammoPos.y + dy }, ammoFontSize, spacing, BLACK);
            }
        }
    }
    //Main ammo text (under outline so its drawn on top)
    DrawTextEx(smallFont, ammoText.c_str(), ammoPos, ammoFontSize, spacing, LIGHTGRAY);

    //Weapon Silhouette (todo)
    DrawTexture(akTexture, 20, screenHeight - akTexture.height - 40, WHITE);
}

void UI::DrawStatic()
{
    DrawTexture(chainTexture, 0, 0, WHITE);
}

