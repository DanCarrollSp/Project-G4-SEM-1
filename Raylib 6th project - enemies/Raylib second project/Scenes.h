#pragma once

#include "raylib.h"

class Scenes
{
public:

	//Constructor and initializer
	Scenes();
	void init();
	bool initOnce = false;

	//Update and draw menus
	void Update();
	void Draw(Camera3D &camera);


	//Scenes
	void updateMainMenu();
	void drawMainMenu(Camera3D &camera);



	//Main menu options
	void NewGame();
	void LoadGame();
	void Options();
	void Quit();



	//Mouse
	Vector2 mousePos;
	//Hover over buttons colors
	Color newGameColor = WHITE;
	Color loadGameColor = WHITE;
	Color optionsColor = WHITE;
	Color quitColor = WHITE;


	/// Scene functons
	void deselectAllScenes();
	/// Scene variables
	//
	//Main menu
	bool sceneMainMenu = true;
	Texture2D titleScreen;
	Texture2D newGame;//New game button
	Rectangle newGameRec;
	float ngWidth = 480;
	Texture2D loadGame;//Load game button
	Rectangle loadGameRec;
	float lgWidth = 510;
	Texture2D options;//Options button
	Rectangle optionsRec;
	float orWidth = 390;
	Texture2D quit;//Quit button
	Rectangle quitRec;
	float qWidth = 220;
	//
	//
	bool sceneGameplay = false;
	bool sceneParticleEngine = false;
};