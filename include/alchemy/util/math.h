#pragma once

#include "alchemy/util/log.h"
#include "alchemy/util/types.h"

#include <math.h>

typedef struct rect
{
    union
    {
        struct
        {
            v2 position;
            v2 size;
        };
        struct
        {
            f32 x;
            f32 y;
            f32 width;
            f32 height;
        };
    };
} rect;

/* General */
inline f32 clamp_f32(f32 value, f32 min, f32 max)
{
    f32 result = value;

    if (value < min)
        result = min;
    else if (value > max)
        result = max;
    
    return result;
}

inline i32 ceil_f32(f32 value)
{
    i32 result = (i32)value;
    if (value != (f32)result && value > 0.0f)
        ++result;
    return result;
}

inline f32 abs_f32(f32 x)
{
    f32 result = fabsf(x);
    return result;
}

inline f32 sq_f32(f32 x)
{
    f32 result = x*x;
    return result;
}

inline f32 sqrt_f32(f32 x)
{
    f32 result = sqrtf(x);
    return result;
}

inline f32 sin_f32(f32 x)
{
    f32 result = sinf(x);
    return result;
}

inline f32 cos_f32(f32 x)
{
    f32 result = cosf(x);
    return result;
}

inline f32 tan_f32(f32 x)
{
    f32 result = tanf(x);
    return result;
}

inline f32 asin_f32(f32 x)
{
    f32 result = asinf(x);
    return result;
}

inline f32 acos_f32(f32 x)
{
    f32 result = acosf(x);
    return result;
}

inline f32 atan_f32(f32 y, f32 x)
{
    ASSERT(x != 0.0f, "Divide by zero");
    f32 result = atan2f(y, x);
    return result;
}

// Convert radians to degrees
inline f32 deg_f32(f32 rad)
{
    f32 result = glm_deg(rad);
    return result;
}

// Convert degrees to radians
inline f32 rad_f32(f32 deg)
{
    f32 result = glm_rad(deg);
    return result;
}

/* v2 */
inline v2 v2_full(f32 fill_value)
{
    v2 result = {fill_value, fill_value};
    return result;
}

inline v2 v2_zero(void)
{
    v2 result = glms_vec2_zero();
    return result;
}

inline v2 v2_one(void)
{
    v2 result = glms_vec2_one();
    return result;
}

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

inline v2 v2_abs(v2 v)
{
    v2 result = v2(abs_f32(v.x), abs_f32(v.y));
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

inline f32 v2_mag(v2 v)
{
    f32 result = sqrt_f32(v2_mag_sq(v));
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

    v2 min = r.position;
    v2 max = v2_add(r.position, r.size);

    if (v.x < min.x) result.x = min.x;
    if (v.y < min.y) result.y = min.y;
    if (v.x > max.x) result.x = max.x;
    if (v.y > max.y) result.y = max.y;

    return result;
}

/* v3 */
inline v3 v3_full(f32 fill_value)
{
    v3 result = {fill_value, fill_value};
    return result;
}

inline v3 v3_zero(void)
{
    v3 result = glms_vec3_zero();
    return result;
}

inline v3 v3_one(void)
{
    v3 result = glms_vec3_one();
    return result;
}

inline v3 v3_add(v3 a, v3 b)
{
    v3 result = glms_vec3_add(a, b);
    return result;
}

inline v3 v3_sub(v3 a, v3 b)
{
    v3 result = glms_vec3_sub(a, b);
    return result;
}

inline v3 v3_neg(v3 v)
{
    v3 result = glms_vec3_negate(v);
    return result;
}

inline v3 v3_abs(v3 v)
{
    v3 result = (v3){abs_f32(v.x), abs_f32(v.y), abs_f32(v.z)};
    return result;
}

inline v3 v3_scale(v3 v, f32 s)
{
    v3 result = glms_vec3_scale(v, s);
    return result;
}

inline f32 v3_dot(v3 a, v3 b)
{
    f32 result = glms_vec3_dot(a, b);
    return result;
}

inline f32 v3_mag_sq(v3 v)
{
    f32 result = v3_dot(v, v);
    return result;
}

inline f32 v3_mag(v3 v)
{
    f32 result = sqrt_f32(v3_mag_sq(v));
    return result;
}

inline v3 v3_reflect(v3 v, v3 r)
{
    v3 result = {0};
    v3 vrr = v3_scale(r, 2.0f*v3_dot(v, r));
    result = v3_sub(v, vrr);
    return result;
}

/* v4 */
inline v4 v4_full(f32 fill_value)
{
    v4 result = {fill_value, fill_value};
    return result;
}

inline v4 v4_zero(void)
{
    v4 result = glms_vec4_zero();
    return result;
}

inline v4 v4_one(void)
{
    v4 result = glms_vec4_one();
    return result;
}

inline v4 v4_add(v4 a, v4 b)
{
    v4 result = glms_vec4_add(a, b);
    return result;
}

inline v4 v4_sub(v4 a, v4 b)
{
    v4 result = glms_vec4_sub(a, b);
    return result;
}

inline v4 v4_neg(v4 v)
{
    v4 result = glms_vec4_negate(v);
    return result;
}

inline v4 v4_abs(v4 v)
{
    v4 result = (v4){abs_f32(v.x), abs_f32(v.y), abs_f32(v.z), abs_f32(v.w)};
    return result;
}

inline v4 v4_scale(v4 v, f32 s)
{
    v4 result = glms_vec4_scale(v, s);
    return result;
}

inline f32 v4_dot(v4 a, v4 b)
{
    f32 result = glms_vec4_dot(a, b);
    return result;
}

inline f32 v4_mag_sq(v4 v)
{
    f32 result = v4_dot(v, v);
    return result;
}

inline f32 v4_mag(v4 v)
{
    f32 result = sqrt_f32(v4_mag_sq(v));
    return result;
}

inline v4 v4_reflect(v4 v, v4 r)
{
    v4 result = {0};
    v4 vrr = v4_scale(r, 2.0f*v4_dot(v, r));
    result = v4_sub(v, vrr);
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

    result.position = min;
    result.size = v2_sub(max, min);

    return result;
}

inline rect rect_center_half_dim(v2 center, v2 half_dim)
{
    rect result = {0};

    result.position = v2_sub(center, half_dim);
    result.size = v2_scale(half_dim, 2.0f);

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

    result.position = min;
    result.size = dim;

    return result;
}

inline b32 rect_is_zero(rect rect)
{
    b32 result = (rect.x == 0.0f && rect.y == 0.0f &&
                  rect.width == 0.0f && rect.height == 0.0f);
    return result;
}

inline rect rect_zero(void)
{
    rect r = {0};
    return r;
}

// NOTE(lucas): Rects are non-inclusive of the max value.
// This allows two rects to perfectly abut and comparison
// will always result in being inside only one rect and not the other.
inline b32 rect_point_in_bounds(rect bounds, v2 test)
{
    v2 min = bounds.position;
    v2 max = v2_add(min, bounds.size);

    b32 result = ((test.x >= min.x) &&
                  (test.y >= min.y) &&
                  (test.x <  max.x) && 
                  (test.y <  max.y));

    return result;
}
