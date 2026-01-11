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

static inline int32_t jc_imin_i32(int32_t a, int32_t b)
{
    return a < b ? a:b;
}

static inline int32_t jc_imax_i32(int32_t a, int32_t b)
{
    return a > b ? a:b;
}


//Edge function: E(a,b,p) = (p.x - a.x) * (b.y - a.y) - (p.y - a.y ) * (b.x - a.x)
//This is the cross product of two 2D vectors, which returns the area.
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

    return (dy < 0) || (dx == 0 && dx > 0);
}

static inline int jc_inside_edge(int64_t e, int top_left)
{
    return (e > 0) || (e == 0 && top_left);
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
    
    //solve for x
    int32_t minx = (minx_fp - JC_FP_HALF + (JC_FP_ONE - 1)) >> JC_FP_SHIFT;
    int32_t maxx = (maxx_fp - JC_FP_HALF) >> JC_FP_SHIFT;
    int32_t miny = (miny_fp - JC_FP_HALF + (JC_FP_ONE - 1)) >> JC_FP_SHIFT;
    int32_t maxy = (maxy_fp - JC_FP_HALF) >> JC_FP_SHIFT;

    //clip to framebuffer
    if(minx < 0) minx = 0;
    if(miny < 0) miny = 0;
    if(maxx > (int32_t) width - 1)  maxx = (int32_t) width - 1;
    if(maxy > (int32_t) height - 1) maxy = (int32_t) height - 1;

    if(minx > maxx || miny > maxy)
    {
        return;
    }

    //edges
    const int32_t e0_dx = v2.x - v1.x;
    const int32_t e0_dy = v2.y - v1.y;
    const int32_t e1_dx = v0.x - v2.x;
    const int32_t e1_dy = v0.y - v2.y;
    const int32_t e2_dx = v1.x - v0.x;
    const int32_t e2_dy = v1.y - v0.y;

    //steps
    const int64_t e0_step_x = (int64_t) e0_dy * (int64_t) JC_FP_ONE;
    const int64_t e1_step_x = (int64_t) e1_dy * (int64_t) JC_FP_ONE;
    const int64_t e2_step_x = (int64_t) e2_dy * (int64_t) JC_FP_ONE;

    const int64_t e0_step_y = -(int64_t) e0_dx * (int64_t) JC_FP_ONE;
    const int64_t e1_step_y = -(int64_t) e1_dx * (int64_t) JC_FP_ONE;
    const int64_t e2_step_y = -(int64_t) e2_dx * (int64_t) JC_FP_ONE;

    JC_V2I p0;
    p0.x = (minx << JC_FP_SHIFT) + JC_FP_HALF;
    p0.y = (miny << JC_FP_SHIFT) + JC_FP_HALF;

    //Start Edge
    int64_t w0_row = jc_edge_i64(v1, v2, p0);
    int64_t w1_row = jc_edge_i64(v2, v0, p0);
    int64_t w2_row = jc_edge_i64(v0, v1, p0);

    const float inv_area = 1.0f / (float) area;

    //scan
    for(int j = (int) miny; j < (int)maxy; j++)
    {
        int64_t w0 = w0_row;
        int64_t w1 = w1_row;
        int64_t w2 = w2_row;

        const int row = j * width;

        for(int i = (int) minx; i < (int)maxx; i++)
        {
            if(jc_inside_edge(w0, tl0) && jc_inside_edge(w1, tl1) && jc_inside_edge(w2, tl2))
            {
                float a = (float) w0 * inv_area;
                float b = (float) w1 * inv_area;
                float c = (float) w2 * inv_area;

                float r = a * v0f.color.r + b * v1f.color.r + c * v2f.color.r;
                float g = a * v0f.color.g + b * v1f.color.g + c * v2f.color.g;
                float bb = a * v0f.color.b + b * v1f.color.b + c * v2f.color.b;
                float aa = a * v0f.color.a + b * v1f.color.a + c * v2f.color.a;

                color[row + i] = jc_pack_rgba8(r, g, bb, aa);
            }

            w0 += e0_step_x;
            w1 += e1_step_x;
            w2 += e2_step_x;
        }
        w0_row += e0_step_y;
        w1_row += e1_step_y;
        w2_row += e2_step_y;
    }


}


