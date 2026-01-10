#pragma once

#include<stdint.h>

typedef struct JC_V2
{
    float x, y;
} JC_V2;

typedef struct JC_Color4
{
    float r, g, b, a;
} JC_Color4;

typedef struct JC_Vertex2D
{
    JC_V2       pos;
    JC_Color4   color;
} JC_Vertex2D;

void jc_raster_tri_color_rgba8(
    uint32_t* color,
    int width,
    int height,
    JC_Vertex2D v0,
    JC_Vertex2D v1,
    JC_Vertex2D v2
);


