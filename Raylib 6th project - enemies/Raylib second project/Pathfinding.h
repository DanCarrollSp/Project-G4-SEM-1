#pragma once

//
#include "raylib.h"
#include <vector>
//
#include <queue>
#include <algorithm>
#include <cmath>


//Struct for a grid node
struct Node
{
    int x;
    int y;
    float gCost;  //Cost from start
    float hCost;  //Heuristic cost to goal
    float fCost;  //gCost + hCost
    Node* parent; //Pointer to previous node

    Node(int xx, int yy) : x(xx), y(yy), gCost(0), hCost(0), fCost(0), parent(nullptr) {}
};



//Computes the A* path (without diagonals) and returns a list of waypoints.
//The returned path is in world coordinates (cells centers).
std::vector<Vector3> AStarPath(Vector3 startPos, Vector3 goalPos,const std::vector<std::vector<bool>>& grid);
//
std::vector<Vector3> SmoothPath(const std::vector<Vector3>& path, const std::vector<std::vector<bool>>& grid);