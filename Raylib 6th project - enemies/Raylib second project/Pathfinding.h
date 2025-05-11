
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
    int z;
    int layer;//Y
    float gCost;  //Cost from start
    float hCost;  //Heuristic cost to goal
    float fCost;  //gCost + hCost
    Node* parent; //Pointer to previous node

    Node(int xx, int zz, int ll) : x(xx), z(zz), layer(ll), gCost(0), hCost(0), fCost(0), parent(nullptr) {}
};


//Stair entry and exit cords
struct StairEdge 
{
    int entryX, entryZ, entryLayer;
    int exitX, exitZ, exitLayer;
};


//Computes the A* path (without diagonals) and returns a list of waypoints.
//The returned path is in world coordinates (cells centers).
std::vector<Vector3> AStarPath(Vector3& startPos, Vector3& goalPos, const std::vector<std::vector<std::vector<bool>>> grid, const std::vector<StairEdge>& stairs);
//
std::vector<Vector3> SmoothPath(const std::vector<Vector3>& path, const std::vector<std::vector<bool>>& grid);