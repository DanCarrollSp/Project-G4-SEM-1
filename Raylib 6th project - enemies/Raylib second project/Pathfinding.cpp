#include "Pathfinding.h"


//Returns Manhattan distance between two grid coords
//Heuristic for grid based movement with no diagonal moves (TODO: Smooth out the movement)
static float ManhattanHeuristic(int x1, int y1, int l1, int x2, int y2, int l2)
{
    //abs(x2 - x1) gives horizontal distance
    //abs(y2 - y1) gives vertical distance
    //Sum is Manhattan distance.
    return (float)(abs(x2 - x1) + abs(y2 - y1) + abs(l2 - l1));
}

//Checks if the given (x , y) coords are within the grid bounds (returns true if it is, false if outside bounds
static bool boundsCheck(int x, int z, int l, const auto& grid)
{
    if (l < 0 || l >= (int)grid.size()) return false;
    if (z < 0 || z >= (int)grid[l].size())  return false;
    if (x < 0 || x >= (int)grid[l][z].size()) return false;
    return true;
}




//Gets the A* path from startPos to goalPos
//startPos and goalPos are world coordinates values
//The grid parameter is a 2D boolean vector containing true(walkable) and false(blocked values)
std::vector<Vector3> AStarPath(Vector3& startPos, Vector3& goalPos, const std::vector<std::vector<std::vector<bool>>> grid, const std::vector<StairEdge>& stairs)
{
    //Converts the world positions to grid coordinates
    //floorf essentially rounds the float values to the nearest int, used to get the grid containing those co0rds
    int startX = (int)floorf(startPos.x);
    int startZ = (int)floorf(startPos.z);
    int startL = int(floorf(startPos.y));
    int goalX = (int)floorf(goalPos.x);
    int goalZ = (int)floorf(goalPos.z);
    int goalL = int(floorf(goalPos.y));

    //Allocate
    int lenght = (int)grid.size();
    int height = (int)grid[0].size();
    int width = grid[0][0].size();


    //Error checking: Ensures both the start and goal positions are in bounds, and that there are walkable grid cells
    if (!boundsCheck(startX, startZ, startL, grid) ||
        !boundsCheck(goalX, goalZ, goalL, grid) ||
        !grid[startL][startZ][startX] ||
        !grid[goalL][goalZ][goalX])
        return {};


    //Creates a 3D grid (vector of vectors) to store pointers to Node objects
    //Each Node is a cell in the grid, storing pathfinding costs and parent pointers
    std::vector<std::vector<std::vector<Node*>>> nodes(lenght,std::vector<std::vector<Node*>>(height,std::vector<Node*>(width, nullptr)));

    //Instantiate a Node for each grid cell
    for (int l = 0; l < lenght; l++)for (int z = 0; z < height; z++)for (int x = 0; x < width; x++)nodes[l][z][x] = new Node(x, z, l);


    //Lambda function to clean up all allocated Node pointers, freeing up memory when done with this current path
    auto cleanup = [&]() {
            for (int l = 0; l < lenght; l++) for (int z = 0; z < height; z++) for (int x = 0; x < width; x++) delete nodes[l][z][x];
        };




    //Gets pointers to the start and goal nodes using grid coords
    Node* startNode = nodes[startL][startZ][startX];
    Node* goalNode = nodes[goalL][goalZ][goalX];



    //Min path using fcost
    //returns true when a's fCost is greater than b’s (so smallest-fCost comes out on top)
    auto compareNodes = [](Node* a, Node* b) { return a->fCost > b->fCost; };
    //1. openSet: Nodes needing evaluation
    //2. closedSet: Nodes that have been evaluated
    std::priority_queue<Node*, std::vector<Node*>, decltype(compareNodes)> openSet(compareNodes);//Capture type
    std::vector<Node*> closedSet;
    openSet.push(startNode);


    //Initialize the start nodes cost values to find paths
    startNode->gCost = 0.0f;//Start with a cost of 0
    startNode->hCost = ManhattanHeuristic(startNode->x, startNode->z, startNode->layer, goalNode->x, goalNode->z, goalNode->layer);
    startNode->fCost = startNode->gCost + startNode->hCost;
    openSet.push(startNode);


    //Defines the four possible movement directions (up , down , left , right)
    //prevents diagonal movement
    const int dirs[4][3] = { {1, 0, 0}, {-1, 0, 0}, {0, 1, 0}, {0, -1, 0} };



    //Main A* loop
    //Continue while there are nodes in the open set
    while (!openSet.empty()) 
    {
        //Pick the next node - the one with lowest fCost
        Node* currentNode = openSet.top(); openSet.pop();

        //Check if weve reached the goal
        if (currentNode == goalNode) 
        {
            //Reconstruct the path if we have
            std::vector<Vector3> path;
            //Convert grid cords back to 3D world cords - centre of the cell
            for (Node* p = goalNode; p; p = p->parent) 
            {
                path.push_back({float(p->x) + 0.5f,float(p->layer),float(p->z) + 0.5f});
            }

            //Reverse our reconstructed goal-to-start path to get start-t-goal
            std::reverse(path.begin(), path.end());

            //Cleanup nodes
            cleanup();
            return path;
        }


        //Mark this node as checked by moving it to the closed set
        closedSet.push_back(currentNode);

        //Check neighbours
        for (auto& direction : dirs) 
        {
            int neighbourX = currentNode->x + direction[0];
            int neighbourZ = currentNode->z + direction[1];
            int neighbourLayer = currentNode->layer + direction[2];

            //Skip if out of bounds or not walkable
            if (!boundsCheck(neighbourX, neighbourZ, neighbourLayer, grid)) continue;
            if (!grid[neighbourLayer][neighbourZ][neighbourX]) continue;


            //Get the gCost (cost from start node to this neighbour node
            Node* neighbourNode = nodes[neighbourLayer][neighbourZ][neighbourX];
            //Skip if already checked
            if (std::find(closedSet.begin(), closedSet.end(), neighbourNode) != closedSet.end()) continue;


            //Get the cost from the start to this neighbour
            float gCostCheck = currentNode->gCost + 1.0f;
            //Better path check
            bool better = (gCostCheck < neighbourNode->gCost) || neighbourNode->parent == nullptr;
            if (better) 
            {
                neighbourNode->parent = currentNode;//Set path to currentNode
                neighbourNode->gCost = gCostCheck;
                neighbourNode->hCost = ManhattanHeuristic(neighbourX, neighbourZ, neighbourLayer, goalX, goalZ, goalL);//Recalc heuristic to goal
                neighbourNode->fCost = neighbourNode->gCost + neighbourNode->hCost;//Total estimated cost
                openSet.push(neighbourNode);//Add neighbour to openset for future exploration in that direction
            }
        }


        //Stair edges
        for (auto& stair : stairs)
        {
            //If current cell is a stair entry point
            if (stair.entryX == currentNode->x && stair.entryZ == currentNode->z && stair.entryLayer == currentNode->layer)
            {
                //Destination of the stair
                int neighbourX = stair.exitX;
                int neighbourZ = stair.exitZ;
                int neighbourLayer = stair.exitLayer;

                //Skip if out of bounds or already checked
                if (!boundsCheck(neighbourX, neighbourZ, neighbourLayer, grid)) continue;
                Node* neighbourNode = nodes[neighbourLayer][neighbourZ][neighbourX];
                if (std::find(closedSet.begin(), closedSet.end(), neighbourNode) != closedSet.end()) continue;


                //Cost of taking stairs (kept ot same amount as normal path
                float gCostCheck = currentNode->gCost + 1.0f;
                //Check if its the better path
                bool better = (gCostCheck < neighbourNode->gCost) || neighbourNode->parent == nullptr;
                if (better)
                {
                    neighbourNode->parent = currentNode;//Set path to currentNode
                    neighbourNode->gCost = gCostCheck;
                    neighbourNode->hCost = ManhattanHeuristic(neighbourX, neighbourZ, neighbourLayer, goalX, goalZ, goalL);//Recalc heuristic to goal
                    neighbourNode->fCost = neighbourNode->gCost + neighbourNode->hCost;//Total estimated cost
                    openSet.push(neighbourNode);//Add neighbour to openset for future exploration in that direction
                }
            }
        }
    }

    //If we exit without finding the goal, there is no path, handling this in the enemy ai by just straight line pathing to playerpos, update it to wander?
    cleanup();
    return {};
}






/// //////////////////////////////////////////////////////////////////////
//Path Smoothing using line of sight

//Checks if a straight line from the start to the end is collision free (Bresenhams algorithm)
bool LineOfSight(const Vector3& startPosition, const Vector3& endPosition, const std::vector<std::vector<bool>>& collisionGrid)
{
    //Converts world coordinates to grid cell indexs
    int startX = static_cast<int>(floorf(startPosition.x));
    int startY = static_cast<int>(floorf(startPosition.z));
    int endX = static_cast<int>(floorf(endPosition.x));
    int endY = static_cast<int>(floorf(endPosition.z));

    //Gets grid dimensions and returns out if the grid is empty
    int gridRows = (int)collisionGrid.size();
    if (gridRows == 0) return false;
    int gridCols = (int)collisionGrid[0].size();


    //Get the horizontal difference between start and end (2D still)
    Vector3 difference = { endPosition.x - startPosition.x, 0.0f, endPosition.z - startPosition.z };
    float distance = sqrtf(difference.x * difference.x + difference.z * difference.z);

    //Sample every micro units to stricten the path towards the original raw grid path center (lower is stricter)
    const float sampleSpacing = 0.0001f;
    int steps = (int)ceilf(distance / sampleSpacing);
    steps = std::max(steps, 1);

    //Walk along the line incrementaly (steps)
    for (int i = 0; i <= steps; ++i) 
    {
        //Calc along the line
        float incrementalSteps = i / (float)steps;
        float sampleX = startPosition.x + difference.x * incrementalSteps;
        float sampleZ = startPosition.z + difference.z * incrementalSteps;

        //Convert to grid cells
        int gridX = (int)floorf(sampleX);
        int gridZ = (int)floorf(sampleZ);

        //If sample is outside grid or a blocked cell no line of sight
        if (gridZ < 0 || gridZ >= gridRows || gridX < 0 || gridX >= gridCols) return false;
        if (!collisionGrid[gridZ][gridX]) return false;
    }
    return true;
}


//Smooths the path by removing unnecessary intermediate waypoints
std::vector<Vector3> SmoothPath(const std::vector<Vector3>& originalPath, const std::vector<std::vector<bool>>& collisionGrid)
{
    //If there are no waypoints, return immediately
    if (originalPath.empty()) return originalPath;

    std::vector<Vector3> smoothedPath;
    smoothedPath.push_back(originalPath.front());//Starting point

    int currentWaypointIndex = 0;
    const int totalWaypoints = static_cast<int>(originalPath.size());

    while (currentWaypointIndex < totalWaypoints - 1)
    {
        //Start by assuming the farthest reachable waypoint is the end of the path
        int furthestReachableIndex = totalWaypoints - 1;

        //Search backwards from the end to find the farthest directly reachable point
        for (int testIndex = totalWaypoints - 1; testIndex > currentWaypointIndex; --testIndex)
        {
            if (LineOfSight(originalPath[currentWaypointIndex], originalPath[testIndex], collisionGrid))
            {
                furthestReachableIndex = testIndex;
                break;
            }
        }

        //Add that reachable waypoint to the smoothed path
        smoothedPath.push_back(originalPath[furthestReachableIndex]);
        currentWaypointIndex = furthestReachableIndex;
    }

    return smoothedPath;
}