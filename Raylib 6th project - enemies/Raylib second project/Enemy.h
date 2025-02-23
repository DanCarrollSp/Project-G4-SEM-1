#ifndef ENEMY_H
#define ENEMY_H

#include "raylib.h"

class Enemy {
public:
	//Constructor and destructor
    Enemy();
    ~Enemy();

	//Main functions
    void Update();
    void Draw(Camera camera);

	//Collision detection
    Vector3 collision(Ray ray);
    BoundingBox hitbox;


private:

    void animate();

    //Position and movement
    Vector3 position;
    float speed;
    //Hitbox
	float hitBoxWidth = 0.23f;
    float hitBoxHeight = 0.5f;

    //Textures and animations
    Texture2D currentTexture;
    Texture2D walkTextures[4];
    bool texturesLoaded = false;
};

#endif // ENEMY_H
