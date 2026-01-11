#pragma once

#include<stdint.h>
#include <math.h>

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

static inline float jc_clamp01(float x)
{
    if (x < 0.0f) return 0.0f;
    if (x > 1.0f) return 1.0f;

    return x;
}

static inline uint32_t jc_pack_rgba8(float r, float g, float b, float a)
{
    r = jc_clamp01(r);
    g = jc_clamp01(g);
    b = jc_clamp01(b);
    a = jc_clamp01(a);

    uint32_t R = (uint32_t) lrint(r * 255.0f);
    uint32_t G = (uint32_t) lrint(g * 255.0f);
    uint32_t B = (uint32_t) lrint(b * 255.0f);
    uint32_t A = (uint32_t) lrint(a * 255.0f);

    return R | (G << 8) | (B << 16) | (A << 24);
}


