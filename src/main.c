#include <stdlib.h>
#include <stdio.h>
#include <float.h>
#include "raylib.h"
#include "raymath.h"
#include "second.h"
#include "Merger.h"
#include "Cube.h"

typedef struct BoardSettings
{
    int width;
    int height;
}BoardSettings;

typedef struct CubeProps CubeProps;
typedef struct CellProps CellProps;
typedef struct Coord2D Coord;
void PlaceCube(CubeProps* props, CellProps* cell);

typedef struct CubeProps
{
    Vector3 position;
    Vector3 cubeSize;
    BoundingBox boundingBox;
    CubeColor color;
    CellProps* locatedCell;
    int level;
    bool isHit;
    bool isDraw;
    bool canSelectable;
}CubeProps;

typedef struct CellProps
{
    Vector3 cellPosition;
    CubeProps* locatedCube;
    bool IsEmpty;
}CellProps;

struct Coord
{
    int x;
    int y;
};

CubeProps* cubeProps = NULL;
CubeProps* currentSelected = NULL;

CellProps* cellArray = NULL;

void InitCubeProps(int width, int height)
{
    cubeProps = (CubeProps*)malloc(sizeof(CubeProps) * (height * width));
    cellArray = (CellProps*)malloc(sizeof(CellProps) * (height * width));
    //MemAlloc()

    const float spacing = 1.8f;
    const float sizeMul = 1.5f;
    float xStart = (width - 1) * (1 + spacing); //1 tile size;
    xStart = (xStart / 2) * -1.0f - 0.5f; //.5f padding because it does not look in the middle
    float yStart = (height - 1) * (1 + spacing);
    yStart = (yStart / 2) * -1.0f;
    Vector3 startPos = (Vector3){
        .x = xStart - sizeMul,
        .y = 0.0f,
        .z = yStart - sizeMul
    };

    for (int i = 0; i < width * height; i++)
    {
        if(i % width == 0)
        {
            startPos.x = xStart;
            startPos.z += sizeMul + spacing;
        }
        else
            startPos.x += sizeMul + spacing;
        

        CubeProps p = (CubeProps)
        {
            .position = startPos,
            .color = Blue,
            .isHit = 0,
            .level = 1,
            .cubeSize = (Vector3){sizeMul, sizeMul, sizeMul},
            .isDraw = true,
            .canSelectable = true
        };
        
        BoundingBox box;
        box.min = (Vector3){startPos.x - p.cubeSize.x / 2 ,
        startPos.y - p.cubeSize.y / 2,
        startPos.z - p.cubeSize.z / 2};
        box.max = (Vector3){startPos.x + p.cubeSize.x / 2 ,
        startPos.y + p.cubeSize.y / 2,
        startPos.z + p.cubeSize.z / 2};

        p.boundingBox = box;

        CellProps tempCell = cellArray[i];
        tempCell.IsEmpty = false;
        tempCell.locatedCube = &cubeProps[i]; 
        tempCell.cellPosition = (Vector3){
            .x = startPos.x,
            .y = startPos.y - 0.5f,
            .z = startPos.z
        };
        p.locatedCell = &cellArray[i];

        cellArray[i] = tempCell;
        cubeProps[i] = p;
    }
}

void DrawCubes(int width, int height, CubeProps* propsArr)
{
    int max = width * height;
    for (int i = 0; i < max; i++)
    {
        CubeProps* temp = &cubeProps[i];
        if ((temp->isDraw == false))
            continue;
        
        if ((temp->isHit == true))
        {
            DrawCube(temp->position, temp->cubeSize.x, temp->cubeSize.y, temp->cubeSize.z, RED);
        }
        else
        {
            DrawCube(temp->position, temp->cubeSize.x, temp->cubeSize.y, temp->cubeSize.z, BLUE);
        }

        if((temp->canSelectable == true))
        {
            BoundingBox box;
            box.min = (Vector3){temp->position.x - temp->cubeSize.x / 2 ,
            temp->position.y - temp->cubeSize.y / 2,
            temp->position.z - temp->cubeSize.z / 2};
            box.max = (Vector3){temp->position.x + temp->cubeSize.x / 2 ,
            temp->position.y + temp->cubeSize.y / 2,
            temp->position.z + temp->cubeSize.z / 2};

            temp->boundingBox = box;
        }
        else
        {
            BoundingBox box = (BoundingBox)
            {
                .min = Vector3Zero(),
                .max = Vector3Zero()
            };
            temp->boundingBox = box;
        }

        DrawBoundingBox(temp->boundingBox, GREEN);
    }
}

void DrawGroundGrids(int width, int height)
{
    const float spacing = 1.8f;
    const float sizeMul = 1.5f;
    float xStart = (width - 1) * (1 + spacing); 
    xStart = (xStart / 2) * -1.0f - 0.5f;
    float yStart = (height - 1) * (1 + spacing);
    yStart = (yStart / 2) * -1.0f;
    Vector3 startPos = (Vector3){
        .x = xStart - sizeMul,
        .y = 0.0f,
        .z = yStart - sizeMul
    };

    for (int i = 0; i < width * height; i++)
    {
        if(i % width == 0)
        {
            startPos.x = xStart;
            startPos.z += sizeMul + spacing;
        }
        else
            startPos.x += sizeMul + spacing;
        
        Vector3 gridPos = (Vector3){
            .x = startPos.x,
            .y = startPos.y - 1.0f,
            .z = startPos.z
        };
        bool b = (cellArray[i].IsEmpty) && (cellArray[i].locatedCube == NULL);
        DrawCube(gridPos, 2.5f, 0.5f, 2.5f, b ? LIME : GRAY);
        DrawCubeWires(gridPos, 2.5f, 0.5f, 2.5f, BLACK);
    }    
}

void CheckIfCubeSelected(Camera cam, int width, int height, CubeProps* propsArr)
{
    if (!IsMouseButtonPressed(MOUSE_BUTTON_LEFT))    
        return;

    printf("\nCheck Collision\n");

    currentSelected = NULL;

    int max = width * height;
    for (int i = 0; i < max; i++)
    {
        CubeProps* props = &propsArr[i];
        if ((props->canSelectable == false) /*|| (props->locatedCell == NULL)*/)
            continue;

        Ray ray = { 0 };                 
        RayCollision collision = { 0 };  

        ray = GetMouseRay(GetMousePosition(), cam);
        collision = GetRayCollisionBox(ray, props->boundingBox);
        props->isHit = collision.hit;

        if ((props->isHit == true)) //&& (props->canSelectable == true)
        {
            currentSelected = &propsArr[i];
        }
        
        printf("%d %d \n", props->isHit, i);
    }
}

void UpdateSelectedPos(Camera cam, BoundingBox box, CubeProps* arr)
{
    if(currentSelected == NULL) return;

    Ray ray = {0};
    RayCollision collision = {0};
    ray = GetMouseRay(GetMousePosition(), cam);
    collision = GetRayCollisionBox(ray, box);

    if ((collision.hit == true))
    {
        currentSelected->position = collision.point;
    }
}

void DrawGroundBoundingBox()
{
    BoundingBox ground;
    ground.min = (Vector3)
    {
        .x = -7,
        .y = 0,
        .z = -12
    };
    ground.max = (Vector3)
    {
        .x = 7,
        .y = 1,
        .z = 12
    };
    DrawBoundingBox(ground, BLACK);
}

void DrawCubeLevelText(Camera cam)
{
    for (int i = 0; i < 3 * 3; i++)
    {
        CubeProps temp = cubeProps[i];
        if ((temp.isDraw == false))
            continue;
        
        Vector3 pos = temp.position;
        pos.x -= 0.5f;
        pos.y += 2.0f;
        Vector2 screenPos = GetWorldToScreen(pos, cam);
        DrawText(TextFormat("%i", temp.level), (int)screenPos.x, (int)screenPos.y, 40, BLACK);
    }
}

bool CanMerge(CubeProps* first, CubeProps* second)
{
    if (first->level != second->level || first->color != second->color)
        return false;

    return true;
}

CellProps* GetClosestCell(Vector3 position)
{
    float dis = FLT_MAX;
    CellProps* result = NULL;
    for (int i = 0; i < 3 * 3; i++)
    {
        CellProps temp = cellArray[i];

        if (Vector3Distance(temp.cellPosition, position) < dis)
        {
            dis = Vector3Distance(temp.cellPosition, position);
            result = &cellArray[i];
        }
    }

    return result;
}

void HandleUp()
{
    if (!IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
        return;

    if (currentSelected == NULL)
        return;
    
    _STATIC_ASSERT(currentSelected != NULL);
    _STATIC_ASSERT(currentSelected->locatedCell != NULL);

    CellProps* closestCell = GetClosestCell(currentSelected->position);
    _STATIC_ASSERT((closestCell != NULL));

    if((closestCell->IsEmpty == true) || (currentSelected == closestCell->locatedCube))
    {
        PlaceCube(currentSelected, closestCell);
    }
    else
    {
        if(CanMerge(currentSelected, closestCell->locatedCube))
        {
            currentSelected->locatedCell->IsEmpty = true;
            currentSelected->locatedCell->locatedCube = NULL;
            currentSelected->locatedCell = NULL;
            currentSelected->isDraw = false;
            currentSelected->canSelectable = false;

            closestCell->locatedCube->isDraw = true;
            closestCell->locatedCube->level += 1;
            closestCell->locatedCube->canSelectable = true;
            closestCell->locatedCube->locatedCell = closestCell;
            closestCell->IsEmpty = false;
        }
        else
        {
            PlaceCube(currentSelected, currentSelected->locatedCell);
        }
    }
    currentSelected->isHit = false;
    currentSelected = NULL;
}

void PlaceCube(CubeProps* cube, CellProps* cell)
{
    cube->locatedCell->IsEmpty = true;
    cube->locatedCell->locatedCube = NULL;

    cell->locatedCube = cube;
    cell->IsEmpty = false;

    cube->locatedCell = cell;
    Vector3 v3 = cell->cellPosition;
    v3.y += 0.5f;
    cube->position = v3;
}

void Callback()
{
    printf("\ncallback\n");
}

void Caller(void(*ptr)())
{
    (*ptr)();
}

int main() 
{
    // Initialization
    //--------------------------------------------------------------------------------------    

    const int screenWidth = 400;
    const int screenHeight = 800;

    BoardSettings settings;
    settings.width  = 3;
    settings.height = 3;
	
    InitWindow(screenWidth, screenHeight, "raylib - color merge game");
    InitCubeProps(settings.width, settings.height);

    Camera camera = { 0 };
    camera.position = (Vector3){
        .x = 0.0f,
        .y = 40.0f,
        .z = 30.0f };
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 30.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------
    
    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        
        //we do not want to cam move
        //UpdateCamera(&camera, CAMERA_ORBITAL);
        //----------------------------------------------------------------------------------
        
        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            BeginMode3D(camera);

            //inputs
            CheckIfCubeSelected(camera, settings.width, settings.height, cubeProps);
            HandleUp();
            BoundingBox ground;
            ground.min = (Vector3)
            {
                .x = -5,
                .y = 0,
                .z = -10
            };
            ground.max = (Vector3)
            {
                .x = 5,
                .y = 1,
                .z = 10
            };
            UpdateSelectedPos(camera, ground, cubeProps);

            //render
            DrawGroundGrids(settings.width, settings.height);
            DrawCubes(settings.width, settings.height, cubeProps);
            DrawGroundBoundingBox();

            EndMode3D();

            DrawCubeLevelText(camera); //not in 3d mode
            DrawText("This is a raylib example", 10, 40, 20, DARKGRAY);

            DrawFPS(10, 10);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------
    free(cubeProps);
    free(cellArray);

    return 0;
}