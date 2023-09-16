#include "util/types.h"

typedef struct rect
{
    v2 min;
    v2 max;
} rect;

/* General */
inline f32 f32_clamp(f32 value, f32 min, f32 max)
{
    f32 result = value;

    if (value < min)
        result = min;
    else if (value > max)
        result = max;
    
    return result;
}

/* v2 */
inline v2 v2_add(v2 a, v2 b)
{
    v2 result = glms_vec2_add(a, b);
    return result;
}

inline v2 v2_sub(v2 a, v2 b)
{
    v2 result = glms_vec2_sub(a, b);
    return result;
}

inline v2 v2_neg(v2 v)
{
    v2 result = glms_vec2_negate(v);
    return result;
}

inline v2 v2_scale(v2 v, f32 s)
{
    v2 result = glms_vec2_scale(v, s);
    return result;
}

inline f32 v2_dot(v2 a, v2 b)
{
    f32 result = glms_vec2_dot(a, b);
    return result;
}

inline f32 v2_mag_sq(v2 v)
{
    f32 result = v2_dot(v, v);
    return result;
}

inline v2 v2_reflect(v2 v, v2 r)
{
    v2 result = {0};
    v2 vrr = v2_scale(r, 2.0f*v2_dot(v, r));
    result = v2_sub(v, vrr);
    return result;
}

inline v2 v2_clamp_to_rect(v2 v, rect r)
{
    v2 result = v;

    if (v.x < r.min.x) result.x = r.min.x;
    if (v.y < r.min.y) result.y = r.min.y;
    if (v.x > r.max.x) result.x = r.max.x;
    if (v.y > r.max.y) result.y = r.max.y;

    return result;
}

/* m4 */

inline m4 m4_identity()
{
    m4 result = {0};
    result = glms_mat4_identity();
    return result;
}

inline m4 m4_ortho(f32 left, f32 right, f32 bottom, f32 top, f32 near_z, f32 far_z)
{
    m4 result = {0};
    result = glms_ortho(left, right, bottom, top, near_z, far_z);
    return result;
}

inline m4 m4_translate(m4 m, v3 v)
{
    m4 result = {0};
    result = glms_translate(m, v);
    return result;
}

inline m4 m4_rotate(m4 m, f32 angle, v3 axis)
{
    m4 result = {0};
    result = glms_rotate(m, angle, axis);
    return result;
}

inline m4 m4_scale(m4 m, v3 v)
{
    m4 result = {0};
    result = glms_scale(m, v);
    return result;
}

/* rect */
inline rect rect_min_max(v2 min, v2 max)
{
    rect result = {0};

    result.min = min;
    result.max = max;

    return result;
}

inline rect rect_center_half_dim(v2 center, v2 half_dim)
{
    rect result = {0};

    result.min = v2_sub(center, half_dim);
    result.max = v2_add(center, half_dim);

    return result;
}

inline rect rect_center_dim(v2 center, v2 dim)
{
    rect result = {0};

    v2 half_dim = v2_scale(dim, 0.5f);
    result = rect_center_half_dim(center, half_dim);

    return result;
}

inline rect rect_min_dim(v2 min, v2 dim)
{
    rect result = {0};

    result.min = min;
    result.max = v2_add(min, dim);

    return result;
}

// NOTE(lucas): Rects are non-inclusive of the max value.
// This allows two rects to perfectly abut and comparison
// will always result in being inside only one rect and not the other.
inline b32 rect_point_in_bounds(rect bounds, v2 test)
{
    b32 result = ((test.x >= bounds.min.x) &&
                  (test.y >= bounds.min.y) &&
                  (test.x <  bounds.max.x) && 
                  (test.y <  bounds.max.y));

    return result;
}
