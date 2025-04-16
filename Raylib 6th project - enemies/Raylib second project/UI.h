#ifndef UI_H
#define UI_H

#include "raylib.h"

#include <string>

class UI 
{
public:
    UI();
    ~UI();

    void LoadAssets(const std::string& fontPath, const std::string& fontSmallPath, const std::string& akImagePath);
    void UnloadAssets();

    void Draw(int health, int currentAmmo, int maxAmmo);
    void DrawStatic();

public:

    //Chain
    Texture2D chainTexture;

    Font mainFont;
    Font smallFont;
    Texture2D akTexture;
    int screenWidth;
    int screenHeight;

    //
    void update();
    //Current weapon
	short pistol = 0;
	short ak = 1;
	short shotgun = 2;
	short smg = 3;

    //Guns
    void reload();
    float reloadTime = 1.0f;//Time it takes to reload
    Texture2D reloadPromptTexture;
    Texture2D reloadingTexture;
    int pistolAmmo = 7;
	int maxPistolAmmo = 7;
    int akAmmo = 30;
	int maxAkAmmo = 30;
    int shotgunAmmo = 2;
	int maxShotgunAmmo = 2;
    int smgAmmo = 40;
	int maxSmgAmmo = 40;

    //pause ui
	Texture2D pauseTexture;
};

#endif // UI_H
