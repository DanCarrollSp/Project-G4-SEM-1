#include "Scenes.h"

Scenes::Scenes()
{

}

void Scenes::init()
{
	//Main menu
	titleScreen = LoadTexture("resources/Menu/Main/TitleScreen.png");
	newGame = LoadTexture("resources/Menu/Main/PLAY.png");
	loadGame = LoadTexture("resources/Menu/Main/PARTICLES.png");
	options = LoadTexture("resources/Menu/Main/LEVELS.png");
	quit = LoadTexture("resources/Menu/Main/QUIT.png");
	//Recs
	newGameRec = { 960 - ngWidth / 2, 355, ngWidth, 100 };
	loadGameRec = { 965 - lgWidth / 2, 530, lgWidth, 100 };
	optionsRec = { 960 - orWidth / 2, 702, orWidth, 100 };
	quitRec = { 968 - qWidth / 2, 870, qWidth, 100 };
}


//Deselects all scenes before selecting a new one
void Scenes::deselectAllScenes()
{
	sceneMainMenu = false;
	sceneGameplay = false;
}

void Scenes::updateMapMaker()
{
}

void Scenes::drawMapMaker(Camera3D&)
{
}


void Scenes::Update()
{
	//Init once
	if (!initOnce)
	{
		init();
		initOnce = true;
	}


	//Mouse
	mousePos = GetMousePosition();

	//Update main menu
	if (sceneMainMenu) updateMainMenu();

}

void Scenes::Draw(Camera3D &camera)
{
	if (WindowShouldClose()) return;

	//Draw main menu
	if (sceneMainMenu) drawMainMenu(camera);
}

void Scenes::updateMainMenu()
{
	/// Highlight buttons when hovering over them
	//main menu
	if (CheckCollisionPointRec(mousePos, newGameRec))
	{
		newGameColor = DARKGRAY;
		if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
		{
			deselectAllScenes();
			sceneGameplay = true;
		}
	}
	else newGameColor = WHITE;
	//load game - Particle editor
	if (CheckCollisionPointRec(mousePos, loadGameRec))
	{
		loadGameColor = DARKGRAY;
		if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
		{
			deselectAllScenes();
			sceneParticleEngine = true;
		}
	}
	else loadGameColor = WHITE;
	//options - level editor
	if (CheckCollisionPointRec(mousePos, optionsRec))
	{
		optionsColor = DARKGRAY;
		if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
		{
			deselectAllScenes();
			sceneMapMaker = true;
		}
	}
	else optionsColor = WHITE;
	//quit
	if (CheckCollisionPointRec(mousePos, quitRec))
	{
		quitColor = DARKGRAY;
		if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) CloseWindow();
	}
	else quitColor = WHITE;
}
void Scenes::drawMainMenu(Camera3D &camera)
{
	BeginDrawing();
	ClearBackground(WHITE);

	//Background
	DrawTexture(titleScreen, 0, 0, WHITE);

	//New Game button
	//DrawRectangleRec(newGameRec, BLACK);
	DrawTexture(newGame, 0, 0, newGameColor);
	//Load Game button
	//DrawRectangleRec(loadGameRec, BLACK);
	DrawTexture(loadGame, 0, 0, loadGameColor);
	//Options button
	//DrawRectangleRec(optionsRec, BLACK);
	DrawTexture(options, 0, 0, optionsColor);
	//Quit button
	//DrawRectangleRec(quitRec, BLACK);
	DrawTexture(quit, 0, 0, quitColor);

	EndDrawing();
}