#ifndef ENEMY_H
#define ENEMY_H

#include "raylib.h"
#include "vector"

class Enemy {
public:
	//Constructor and destructor
    Enemy();
    ~Enemy();

    //Sets position after construction
    void SetPosition(const Vector3& newPos);


	//Main functions
    void Update();
    void Draw(Camera camera);

	//Collision detection
	Vector3 collision(Ray ray);//Returns collision point if a ray hits the enemy (used for bullets)
	void vectorCollision(const std::vector<BoundingBox>& walls);//Gives enemy phycial collisions with walls
    BoundingBox hitbox;
    BoundingBox GetBoundingBox() const { return hitbox; }

    //Movement and path finding
    void Move(Vector3 target, const std::vector<std::vector<bool>>& navGrid, const std::vector<BoundingBox>& walls, float deltaTime);
    void RecalculatePath(Vector3 target, const std::vector<std::vector<bool>>& navGrid);
    //Path data
    std::vector<Vector3> gridPath;//The calculated path to target
	std::vector<Vector3> smoothPath;//The smoothed path to target
    int currentPathIndex;//Part of the path its currently on
	float timeSinceLastPathRecalc;//Timer to wait before trying to find a new path again
    Vector3 lastTargetPosition;//Position to move to

    
    //Get functions
	Vector3 GetPosition() const { return position; }
    bool debug = false;

    // New methods for health
    void TakeDamage(int amount);
    bool IsAlive() const { return isAlive; }

private:

	//Animation of enemy movements
    void animate();
    double lastAnimationTime = 0.0;
    int frameIndex = 0;

    //Position and movement
    float speed;
    Vector3 position;
    // Save the last frame position to check for movement for animating
    Vector3 lastPosition;

    //Hitbox
	float hitBoxWidth = 0.23f;
    float hitBoxHeight = 0.5f;

    //Textures and animations
    Texture2D currentTexture;
    Texture2D walkTextures[4];
    bool texturesLoaded = false;

    // Health & alive status
    int health;
    bool isAlive;
};

#endif // ENEMY_H