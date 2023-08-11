#ifndef CUBE_H_
#define CUBE_H_

typedef enum CubeColor
{
    Blue = 0, Red = 1, Yellow = 2, Green = 3, Violet = 4, Orange = 5, None = 6
}CubeColor;

typedef struct Cube
{
    CubeColor cubeColor;
    int level;
}Cube;

#endif