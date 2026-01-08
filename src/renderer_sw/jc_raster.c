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
static inline int64_t jc_edge_i64(JC_V2I a, JC_V2I b, JC_V2I p)
{
    int64_t pax = (int64_t)p.x - (int64_t)a.x;
    int64_t bay = (int64_t)b.y - (int64_t)a.y;

    int64_t pay = (int64_t)p.y - (int64_t)a.y;
    int64_t bax = (int64_t)b.x - (int64_t)a.x;

    return pax * bay - pay * bax;
}

