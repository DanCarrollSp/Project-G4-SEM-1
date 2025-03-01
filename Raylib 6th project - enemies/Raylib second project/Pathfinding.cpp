#include "Pathfinding.h"


//Returns Manhattan distance between two grid coords
//Heuristic for grid based movement with no diagonal moves (TODO: Smooth out the movement)
static float ManhattanHeuristic(int x1, int y1, int x2, int y2)
{
    //abs(x2 - x1) gives horizontal distance
    //abs(y2 - y1) gives vertical distance
    //Sum is Manhattan distance.
    return (float)(abs(x2 - x1) + abs(y2 - y1));
}

//Checks if the given (x , y) coords are within the grid bounds (returns true if it is, false if outside bounds
static bool boundsCheck(int x, int y, int width, int height)
{
    return (x >= 0 && x < width && y >= 0 && y < height);
}




//Gets the A* path from startPos to goalPos
//startPos and goalPos are world coordinates values
//The grid parameter is a 2D boolean vector containing true(walkable) and false(blocked values)
std::vector<Vector3> AStarPath(Vector3 startPos, Vector3 goalPos, const std::vector<std::vector<bool>>& grid)
{
    //Converts the world positions to grid coordinates
    //floorf essentially rounds the float values to the nearest int, used to get the grid containing those co0rds
    int startX = (int)floorf(startPos.x);
    int startY = (int)floorf(startPos.z);
    int goalX = (int)floorf(goalPos.x);
    int goalY = (int)floorf(goalPos.z);

    //Determine the grid dimensions.
    int height = (int)grid.size();
    if (height == 0) return {};//Return an empty path if the grid has no rows
    int width = (int)grid[0].size();


    //Error checking: Ensures both the start and goal positions are in bounds, and that there are walkable grid cells.
    if (!boundsCheck(startX, startY, width, height) || !boundsCheck(goalX, goalY, width, height) || !grid[startY][startX] || !grid[goalY][goalX]) return {};


    //Creates a 2D grid (vector of vectors) to store pointers to Node objects
    //Each Node is a cell in the grid, storing pathfinding costs and parent pointers
    std::vector<std::vector<Node*>> nodeGrid(height, std::vector<Node*>(width, nullptr));

    //Instantiate a Node for each grid cell
    for (int y = 0; y < height; y++) for (int x = 0; x < width; x++) nodeGrid[y][x] = new Node(x, y);


    //Lambda function to clean up all allocated Node pointers, freeing up memory when done with this current path
    auto cleanup = [&]()
        {
            for (int y = 0; y < height; y++) for (int x = 0; x < width; x++) delete nodeGrid[y][x];

        };




    //Gets pointers to the start and goal nodes using grid coords
    Node* startNode = nodeGrid[startY][startX];
    Node* goalNode = nodeGrid[goalY][goalX];

    //Initialize two sets: 
    //1. openSet: Nodes needing evaluation
    //2. closedSet: Nodes that have been evaluated
    std::vector<Node*> openSet;
    std::vector<Node*> closedSet;
    openSet.push_back(startNode);

    //Initialize the start nodes cost values to find paths
    startNode->gCost = 0.0f;//Start with a cost of 0
    startNode->hCost = ManhattanHeuristic(startNode->x, startNode->y, goalNode->x, goalNode->y);
    startNode->fCost = startNode->gCost + startNode->hCost;


    //Defines the four possible movement directions (up , down , left , right)
    //prevents diagonal movement
    const int dirs[4][2] = { {1, 0}, {-1, 0}, {0, 1}, {0, -1} };



    //Main A* loop
    //Continue while there are nodes in the open set
    while (!openSet.empty())
    {
        //Sorts the open set by fCost (lowest first)
        std::sort(openSet.begin(), openSet.end(), [](Node* a, Node* b) { return a->fCost < b->fCost; });

        //The node with the smallest fCost is the current node
        Node* current = openSet.front();
        //Removes current node from the open set
        openSet.erase(openSet.begin());

        //Check if the current node is the goal
        if (current == goalNode)
        {
            //If goal is reached, reconstruct the path from goal to start by following the parent pointers
            std::vector<Vector3> path;
            Node* temp = goalNode;
            while (temp != nullptr)
            {
                //Converts grid coordinates back to world coordinates
                //Adding 0.5f to center the position in the cell
                Vector3 pos;
                pos.x = (float)temp->x + 0.5f;
                //pos.y = startPos.y;//Can set the y pos for future proofing
                pos.z = (float)temp->y + 0.5f;
                path.push_back(pos);
                //Move to the parent node
                temp = temp->parent;
            }

            //The path constructs from goal to start, reverse it to get the correct order
            std::reverse(path.begin(), path.end());

            
            cleanup();//Clean up memory
            return path;//Return the reconstructed path.
        }


        //Adds the current node to the closed set  to mark it as checked
        closedSet.push_back(current);

        //Loops through all valid neighbouring cells (4 directional movement)
        for (auto& d : dirs)
        {
            //Calculates neighbour coordinates
            int nx = current->x + d[0];
            int ny = current->y + d[1];

            //Skip if its out of bounds
            if (!boundsCheck(nx, ny, width, height))
                continue;
            //Skip if the cell is not walkable
            if (!grid[ny][nx])
                continue;


            //Get the neighbour node from the node grid
            Node* neighbour = nodeGrid[ny][nx];

            //If the neighbour has already been evaluated skip it
            if (std::find(closedSet.begin(), closedSet.end(), neighbour) != closedSet.end()) 
                continue;

            //Calculate the gCost for the neighbour
            //Adding 1.0f because moving from one cell to an adjacent cell costs 1
            float newGCost = current->gCost + 1.0f;

            //If the neighbour is not in the openSet (not discovered) OR if the new path to the neighbour is cheaper than a previously discovered path then -
            if (std::find(openSet.begin(), openSet.end(), neighbour) == openSet.end() || newGCost < neighbour->gCost)
            {
                //Update the neighbour cost values
                neighbour->gCost = newGCost;
                neighbour->hCost = ManhattanHeuristic(neighbour->x, neighbour->y, goalNode->x, goalNode->y);
                neighbour->fCost = neighbour->gCost + neighbour->hCost;
                //Set the current node as the neighbours parent for path reconstruction
                neighbour->parent = current;

                //If the neighbour was not already in openSet - adds it
                if (std::find(openSet.begin(), openSet.end(), neighbour) == openSet.end()) openSet.push_back(neighbour);
            }
        }
    }

    //If the loop ends without finding a path to the goal clean up and return an empty path
    cleanup();
    return {};
}






//-----------------------------------------------------------------------------
// Path Smoothing Functions (Line-of-Sight "String Pulling")
//-----------------------------------------------------------------------------

// Checks if a straight line from start to end is collision-free using Bresenham's algorithm.
bool LineOfSight(const Vector3& start, const Vector3& end, const std::vector<std::vector<bool>>& grid) {
    int x0 = (int)floorf(start.x);
    int y0 = (int)floorf(start.z);
    int x1 = (int)floorf(end.x);
    int y1 = (int)floorf(end.z);

    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;

    while (true) {
        if (!grid[y0][x0])
            return false;
        if (x0 == x1 && y0 == y1)
            break;
        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }
    }
    return true;
}

// Smooths the computed path by removing unnecessary intermediate waypoints.
std::vector<Vector3> SmoothPath(const std::vector<Vector3>& path, const std::vector<std::vector<bool>>& grid) {
    if (path.empty())
        return path;

    std::vector<Vector3> newPath;
    newPath.push_back(path.front());

    int currentIndex = 0;
    while (currentIndex < (int)path.size() - 1) {
        int nextIndex = (int)path.size() - 1;
        // Look for the farthest point reachable in a straight line.
        for (int i = (int)path.size() - 1; i > currentIndex; i--) {
            if (LineOfSight(path[currentIndex], path[i], grid)) {
                nextIndex = i;
                break;
            }
        }
        newPath.push_back(path[nextIndex]);
        currentIndex = nextIndex;
    }
    return newPath;
}
