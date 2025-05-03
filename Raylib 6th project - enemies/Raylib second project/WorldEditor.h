#pragma once

//Raylib includes
#include "raylib.h"
#include "raymath.h"
//Global includes
#include <filesystem>
#include <random>
//Local includes
#include "Globals.h"


//Data structure for each of the maps
struct Map
{
    int width = 0, height = 0, levels = 0;
    std::vector<Color> voxels;
};


//Block data
enum Block
{
    //Basic
    BLK_WALL,
    BLK_DOOR1,
    BLK_DOOR2,
    BLK_FLOOR,
    BLK_STONE,
    BLK_TEST,

    //Utility
    BLK_SPAWN,

    //Stairs
    BLK_STAIR_N,
    BLK_STAIR_S,
    BLK_STAIR_E,
    BLK_STAIR_W,

    //Brick
    BLK_BRICK1,
    BLK_BRICK2,
    BLK_BRICK3,
    BLK_BRICK4,

    //Brick Faces
    BLK_BRICK_FACE1,
    BLK_BRICK_FACE2,
    BLK_BRICK_FACE2_BLOOD,
    BLK_BRICK_FACE3,
    BLK_BRICK_FACE3_BLOOD,
    BLK_BRICK_FACE4,
    BLK_BRICK_FACE5,
    BLK_RED_BRICK,

    //Cement / Concrete
    BLK_CEMENT_WALL,
    BLK_CEMENT_BOTTOM1,
    BLK_CEMENT_BOTTOM2,
    BLK_CEMENT_BOTTOM3,
    BLK_CEMENT_DECO1,
    BLK_CEMENT_DECO2,
    BLK_CEMENT_TOP1,
    BLK_CEMENT_TOP2,

    //Dark Steel
    BLK_DARK_STEEL,
    BLK_DARK_STEEL_BEAM,
    BLK_DARK_STEEL_DECO,
    BLK_DARK_STEEL_HAZARD,
    BLK_DARK_STEEL_TOP1,
    BLK_DARK_STEEL_TOP2,
    BLK_DARK_STEEL_WALL,
    BLK_DARK_STEEL_WALL_DECO,

    //Steel
    BLK_OLD_STEEL,
    BLK_RUST_BEAM_UP,
    BLK_RUST_BEAM_SIDE,
    BLK_STEEL_BEAM1,
    BLK_STEEL_BEAM2,
    BLK_STEEL_DOOR,
    BLK_STEEL_GRIP1,
    BLK_STEEL_GRIP2,
    BLK_STEEL_PLATE,
    BLK_STEEL_REINFORCED,

    //Grates
    BLK_GRATE,
    BLK_GRATE_RUST,

    //Misc
    BLK_SUPPORT_BEAM,
    BLK_SWITCH_GREEN,
    BLK_SWITCH_RED,
    BLK_BIG_DOOR_LEFT1,
    BLK_BIG_DOOR_RIGHT1,
    BLK_BIG_DOOR_LEFT2,
    BLK_BIG_DOOR_RIGHT2,
    BLK_FENCE,
    BLK_VENT,
};
//Block info for the palette
struct BlockInfo
{
    Block type;//Which block
    Texture2D texture;//Cube‐face texture
    const char* name;//Name of the block for ui (raylib requires const chat* for DrawText(), but this is basically a string)
    Rectangle rect;//The rectangle for the block in the palette
};


class WorldEditor
{
private:



public:

    //Constructor, reads pngs from maps dir and populates the editor with its info
    WorldEditor();
    void Init();//Inits the palette assets and positions


    //Draws the 3D world and the UI
    void Draw3D(Camera3D&);
    Shader alphaShader;
    //Update and handle user inputs for the editor
    void HandleInput(Camera3D&);


    //Map creation, loading and saving to and from files
    void NewBlank(int w, int h, int lvls);
    bool LoadMap(int index);
    bool SaveMap(int index) const;


    //Takes in all the placed player spawn points in the level and randomly selects a spawn point
    bool FindPlayerSpawn(Vector3& out);


    //Converts placed voxels to pixels for the image
    Image BakeImage();



private:

    //Initial vars
    int currentLevel = 0;//Ground level, default to 0 if not set
    Block current = BLK_WALL;

    //Init funcs
    void initPaletteTextures();
    void InitPalette();
    void InitCommands();//Builds the list of buttons

    //Helpers
    // 
    //Helper function to turn 3D Map coordinates into an index for the 1D vector
    int coord3dToIndex(int layer, int row, int col)
    {
        return layer * maps[currentMapIndex].width * maps[currentMapIndex].height + row * maps[currentMapIndex].width + col;
    }
    //
    //Helper function to check if a set of coordinates are within the bounds of the map (height level, width and length)
    bool inside(int layer, int row, int col)
    {
        return layer >= 0 && layer < maps[currentMapIndex].levels &&
            row >= 0 && row < maps[currentMapIndex].height &&
            col >= 0 && col < maps[currentMapIndex].width;
    }



    //Map data
    std::vector<Map> maps;//Vector of the voxels/tiles
    std::vector<std::string> mapPaths;//Vector of the dir map paths to the pngs
    int currentMapIndex = 0;//Current map were on

    //Changes the current map to the one selected by the user (saves the current map to file then moves onto the next one, then loads it from file in case it hasnt loaded correctly)
    void SwitchToMap(int newIndex);
    //Gets the default path for the map
    std::string DefaultMapPath(int i);

    //Block placement brush
    int lastX{ -1 }, lastY{ -1 }, lastZ{ -1 };//Last voxel placed this draw (for continuing a line)
    int lastBtn{ -1 };//Which mouse button was used last (for stopping drag drawing when only placing once)
    int dragLayer = -1;//Layer to place on when dragging

    //Returns color of the pixel color of the placed block (used to convert the block to a pixel for the save file image)
    Color BlockToPixel(Block);


    ///Undo and Redo
    //Index value used to mark the start/end of a grouped edit for undo and redo
    int placementGroupMarker = 0;
    //Allows for undoing and redoing of single or multiple edits using command buttons or shortcuts
    void undo();
    void redo();

    //Edit history identification
    struct Edit
    {
        int index;//voxel linear index
        Color before;//value that was there
        Color after;//value to be set
    };
    //Edit history storage
    std::vector<Edit> undoStack;
    std::vector<Edit> redoStack;
    std::size_t maxEditHistory = 10000;//Max history size



    //Block Palette stuff
    bool paletteExpanded = false;//Bool to check if the palette is open or closed
    Rectangle paletteToggle;//The little box to open/close the palette

    //Populated with the block textures and names
    std::vector<BlockInfo> palette;

    //Palette scroll bar and handle
    void UpdatePaletteScroll(Vector2& mousePos, float wheelDelta);
    float paletteScroll = 0.0f;
    bool paletteScrolling = false;
    Rectangle paletteScrollBar;
    Rectangle paletteScrollHandle;

    //Drawing and updating the palette
    void DrawPalette();



    //Command Palette stuff
    bool cmdsPaletteExpanded = false;//Bool to check if the palette is open or closed
    Rectangle cmdsPaletteToggle;//The little box to open/close the palette

    //Command buttons
    enum CmdID
    {
        CMD_Save, CMD_Load,//File saving and loading 

        CMD_Undo, CMD_Redo,//Edit history
        CMD_ClearLayer, CMD_ClearTiles,//Clearing helpers
        CMD_NextMap, CMD_PrevMap,//Map navigation
        CMD_OpenAllMaps,//File refreshing
        CMD_NewMap//Map creation
    };
    struct CmdButton
    {
        Rectangle rect;
        const char* label;
        CmdID id;
    };
    std::vector<CmdButton> commandButtons;


    void DrawCommands();//Draws commands
    void DrawCommandPalette();//Draws the commands toggle button, and if expanded also draws the command buttons
    bool CheckCommandClicks();//Returns true if click was inside the command UI


    //Does one command if clicked, by id
    void DoCommand(CmdID);
    void ClearCurrentLayer();//Clears all current maps tiles on the current layer
    void ClearTiles();//Clears all current maps tiles
};