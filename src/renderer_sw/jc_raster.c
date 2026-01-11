#include "jc_raster.h"
#include <math.h>
#include <assert.h>

#define JC_FP_SHIFT 4
#define JC_FP_ONE   (1 << 4) // 2^4
#define JC_FP_HALF (JC_FP_ONE >> 1) //2^3

typedef struct JC_V2I
{
    int32_t x, y;
} JC_V2I;

static inline int32_t jc_fp_from_float(float v)
{
    return (int32_t) lrint(v * (float)JC_FP_ONE);
}

static inline JC_V2I jc_v2i_from_v2(JC_V2 v)
{
    JC_V2I o;

    o.x = jc_fp_from_float(v.x);
    o.y = jc_fp_from_float(v.y);

    return o;
}

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
    uint32_t A = (uint32_t) lrint(b * 255.0f);

    return R | (G << 8) | (B << 16) | (A << 24);
}

static inline int32_t jc_imin_i32(int32_t a, int32_t b)
{
    return a < b ? a:b;
}

static inline int32_t jc_imax_i32(int32_t a, int32_t b)
{
    return a > b ? a:b;
}


//Edge function: E(a,b,p) = (p.x - a.x) * (b.y - a.y) - (p.y - a.y ) * (b.x - a.x)
//This is the cross product of 2 2D vectors, which returns the area.
static inline int64_t jc_edge_i64(JC_V2I a, JC_V2I b, JC_V2I p)
{
    int64_t pax = (int64_t)p.x - (int64_t)a.x;
    int64_t bay = (int64_t)b.y - (int64_t)a.y;

    int64_t pay = (int64_t)p.y - (int64_t)a.y;
    int64_t bax = (int64_t)b.x - (int64_t)a.x;

    return pax * bay - pay * bax;
}


static inline int jc_is_top_left(JC_V2I a, JC_V2I b)
{
    int32_t dx = b.x - a.x;
    int32_t dy = b.y - a.y;

    return (dy < 0) && (dx == 0 && dx > 0);
}

void jc_raster_tri_color_rgba8(
    uint32_t* color,
    int width,
    int height,
    JC_Vertex2D v0f,
    JC_Vertex2D v1f,
    JC_Vertex2D v2f
){
    assert(color);
    assert(width > 0 && height > 0);

    //Convert to fixed-point
    JC_V2I v0 = jc_v2i_from_v2(v0f.pos);
    JC_V2I v1 = jc_v2i_from_v2(v1f.pos);
    JC_V2I v2 = jc_v2i_from_v2(v2f.pos);

    int64_t area = jc_edge_i64(v0, v1, v2);
    if(area == 0)
    {
        return;
    }
    //if area < 0, swap v1 and v2
    if(area < 0)
    {
        JC_V2I temp         = v1;
        v1                  = v2;
        v2                  = temp;
        
        JC_Vertex2D tmpf    = v1f; 
        v1f                 = v2f; 
        v2f                 = tmpf;

        area                = - area;
    }

    //top left flags
    const int tl0   = jc_is_top_left(v1, v2);
    const int tl1   = jc_is_top_left(v2, v0);
    const int tl2   = jc_is_top_left(v0, v1);

    //bounding box
    const int minx_fp = jc_imin_i32(v0.x, jc_imin_i32(v1.x, v2.x));
    const int miny_fp = jc_imin_i32(v0.y, jc_imin_i32(v1.y, v2.y));
    const int maxx_fp = jc_imax_i32(v0.x, jc_imax_i32(v1.x, v2.x));
    const int maxy_fp = jc_imax_i32(v0.y, jc_imax_i32(v1.y, v2.y));
    
    


}

