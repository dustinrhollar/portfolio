#ifndef _MAPLE_MATH_H
#define _MAPLE_MATH_H

#ifndef MM_PI
#define MM_PI 3.141592653589793238f 
#endif

typedef union
{
    r32 p[2];
    struct { r32 x, y; };
} Vec2;

typedef union
{
    r32 p[3];
    struct { r32 x, y, z;     };
    struct { Vec2 xy; r32 p0; };
    struct { r32 p1; Vec2 yz; };

    struct { r32 r, g, b;     };
    struct { Vec2 rg; r32 p2; };
    struct { r32 p3; Vec2 gb; };
} Vec3;

typedef union
{
    r32 p[4];
    struct { r32 x, y, z, w;          };
    struct { Vec2 xy; Vec2 zw;        };
    struct { r32 p0; Vec2 yz; r32 p1; };
    struct { Vec3 xyz; r32 p2;        };
    struct { r32 p3; Vec3 yzw;        };

    struct { r32 r, g, b, a;          };
    struct { Vec2 rg; Vec2 ba;        };
    struct { r32 p4; Vec2 gb; r32 p5; };
    struct { Vec3 rgb; r32 p6;        };
    struct { r32 p7; Vec3 gba;        };
} Vec4;

typedef union 
{
    r32 p[3][3];
    struct { Vec3 c0, c1, c2; };
} Mat3;

typedef union 
{
    r32 p[4][4];
    struct { Vec4 c0, c1, c2, c3; };
} Mat4;

typedef union
{
    struct { r32 x, y, z, w;   };
    struct { Vec3 xyz; r32 p0; };
} Quaternion;

typedef Vec2       v2;
typedef Vec3       v3;
typedef Vec4       v4;
typedef Vec3       c3; // 3 component color (rgb)
typedef Vec4       c4; // 4 component color (rgba)
typedef Mat3       m3;
typedef Mat4       m4;
typedef Quaternion qt;

#define V2_ZERO { .x = 0.0f, .y = 0.0f }
#define V3_ZERO { .x = 0.0f, .y = 0.0f, .z = 0.0f}
#define V4_ZERO { .x = 0.0f, .y = 0.0f, .z = 0.0f, .w = 0.0f }
#define V2_ONE  { .x = 1.0f, .y = 1.0f }
#define V3_ONE  { .x = 1.0f, .y = 1.0f, .z = 1.0f }
#define V4_ONE  { .x = 1.0f, .y = 1.0f, .z = 1.0f, .w = 1.0f }

#define M3_IDENTITY m3_diag(1.0f) 
#define M4_IDENTITY m4_diag(1.0f) 

FORCE_INLINE r32 degrees_to_radians(r32 theta)
{
    return theta * (MM_PI / 180.0f);
}

FORCE_INLINE r32 lerp(r32 v0, r32 v1, r32 t);
FORCE_INLINE r32 inv_lerp(r32 v0, r32 v1, r32 v);
FORCE_INLINE r32 remap(r32 i0, r32 i1, r32 o0, r32 o1, r32 v);

// Vec2 Pre-decs

static v2  v2_add(v2 left, v2 right);
static v2  v2_sub(v2 left, v2 right);
static v2  v2_mul(v2 left, v2 right);
static v2  v2_mulf(v2 left, r32 right);
static v2  v2_divf(v2 left, r32 right);
static r32 v2_dot(v2 left, v2 right);
/* Computes z-component in 3D space on an xy plane */
static r32 v2_cross(v2 left, v2 right);
static v2  v2_norm(v2 left);
static r32 v2_mag(v2 left);
static r32 v2_mag_sq(v2 left);

// Vec3 Pre-decs

static v3  v3_add(v3 left, v3 right);
static v3  v3_sub(v3 left, v3 right);
static v3  v3_mul(v3 left, v3 right);
static v3  v3_mulf(v3 left, r32 right);
static v3  v3_divf(v3 left, r32 right);
static r32 v3_dot(v3 left, v3 right);
static v3  v3_cross(v3 left, v3 right);
static v3  v3_norm(v3 left);
static r32 v3_mag(v3 left);
static r32 v3_mag_sq(v3 left);

// Vec4 Pre-decs

static v4  v4_add(v4 left, v4 right);
static v4  v4_sub(v4 left, v4 right);
static v4  v4_mul(v4 left, v4 right);
static v4  v4_mulf(v4 left, r32 right);
static v4  v4_divf(v4 left, r32 right);
static r32 v4_dot(v4 left, v4 right);
/* Takes the cross product of xyz components and sets w = 1.0f */
static v4  v4_cross(v4 left, v4 right);
static v4  v4_norm(v4 left);
static r32 v4_mag(v4 left);
static r32 v4_mag_sq(v4 left);

// MAT3 Pre-decs

static m3 m3_diag(r32 d);

// MAT4 Pre-decs

static m4 m4_diag(r32 d);
static m4 m3_to_m4(m3 mat);
static m4 m4_mul(m4 left, m4 right);
/* Creates a scaling matrix */
static m4 m4_scale(r32 sx, r32 sy, r32 sz);
/* Creates a translation matrix */
static m4 m4_translate(v3 trans);
/* Creates a look at matrix */
static m4 m4_look_at(v3 eye, v3 center, v3 up);
/* Creates a perspective projection matrix */
static m4 m4_perspective(r32 fov, r32 ar, r32 near, r32 far);
static m4 m4_rotate_x(r32 theta);
static m4 m4_rotate_y(r32 theta);
static m4 m4_rotate_z(r32 theta);
static m4 m4_rotate(r32 theta, v3 axis);


// Other Utility Pre-decs
static r32 clamp(r32 min, r32 max, r32 val);
static r32 smoothstep(r32 v0, r32 v1, r32 t);
static r32 smootherstep(r32 v0, r32 v1, r32 t);

#endif //_MAPLE_MATH_H

#if defined(MAPLE_MATH_IMPLEMENTATION)

FORCE_INLINE
r32 lerp(r32 v0, r32 v1, r32 t)
{
    return v0 + t * (v1 - v0);
}

FORCE_INLINE
r32 inv_lerp(r32 v0, r32 v1, r32 v)
{
    r32 Result = 0.0f;
    
    Result = (v - v0) / (v1 - v0);
    Result = (Result < 0.0f) ? 0.0f : (Result > 1.0f) ? 1.0f : Result;
    
    return Result;
}

FORCE_INLINE
r32 remap(r32 i0, r32 i1, r32 o0, r32 o1, r32 v)
{
    r32 t = inv_lerp(i0, i1, v);
    return lerp(o0, o1, t);
}

// Vec2 Pre-decs

static v2 v2_add(v2 left, v2 right)
{
    v2 result;

    result.x = left.x + right.x;
    result.y = left.y + right.y;

    return result;
}

static v2 v2_sub(v2 left, v2 right)
{
    v2 result;

    result.x = left.x - right.x;
    result.y = left.y - right.y;

    return result;
}

static v2 v2_mul(v2 left, v2 right)
{
    v2 result;

    result.x = left.x * right.x;
    result.y = left.y * right.y;

    return result;
}

static v2 v2_mulf(v2 left, r32 right)
{
    v2 result;

    result.x = left.x * right;
    result.y = left.y * right;

    return result;
}

static v2 v2_divf(v2 left, r32 right)
{
    assert(right != 0.0f && "Attempted to divide a Vec2 by 0.0f");

    v2 result;

    result.x = left.x / right;
    result.y = left.y / right;

    return result;
}

static r32 v2_dot(v2 left, v2 right)
{
    r32 result = 0.0f;

    result += left.x * right.x;
    result += left.y * right.y;

    return result;
}

static r32 v2_cross(v2 left, v2 right)
{
    r32 result;
    result = (left.x * right.y) - (left.y * right.x);
    return result;
}

static v2 v2_norm(v2 left)
{
    v2 result;

    r32 mag = 1.0f / v2_mag(left);
    result.x = left.x * mag;
    result.y = left.y * mag;

    return result;
}

static r32 v2_mag(v2 left)
{
    r32 result;
    result = sqrtf(left.x * left.x + left.y * left.y);
    return result;
}

static r32 v2_mag_sq(v2 left)
{
    r32 result;
    result = left.x * left.x + left.y * left.y;
    return result;
}

// Vec3 Pre-decs

static v3 v3_add(v3 left, v3 right)
{
    v3 result;

    result.x = left.x + right.x;
    result.y = left.y + right.y;
    result.z = left.z + right.z;

    return result;
}

static v3 v3_sub(v3 left, v3 right)
{
    v3 result;

    result.x = left.x - right.x;
    result.y = left.y - right.y;
    result.z = left.z - right.z;

    return result;
}

static v3 v3_mul(v3 left, v3 right)
{
    v3 result;

    result.x = left.x * right.x;
    result.y = left.y * right.y;
    result.z = left.z * right.z;

    return result;
}

static v3 v3_mulf(v3 left, r32 right)
{
    v3 result;

    result.x = left.x * right;
    result.y = left.y * right;
    result.z = left.z * right;

    return result;
}

static v3 v3_divf(v3 left, r32 right)
{
    assert(right != 0.0f && "Attempted to divide a Vec2 by 0.0f");

    v3 result;

    result.x = left.x / right;
    result.y = left.y / right;
    result.z = left.z / right;

    return result;
}

static r32 v3_dot(v3 left, v3 right)
{
    r32 result = 0.0f;

    result += left.x * right.x;
    result += left.y * right.y;
    result += left.z * right.z;

    return result;
}

static v3 v3_cross(v3 left, v3 right)
{
    v3 result;

    result.x = (left.y * right.z) - (left.z * right.y);
    result.y = (left.z * right.x) - (left.x * right.z);
    result.z = (left.x * right.y) - (left.y * right.x);
    
    return result;
}

static v3 v3_norm(v3 left)
{
    v3 result;

    r32 mag = 1.0f / v3_mag(left);
    result.x = left.x * mag;
    result.y = left.y * mag;
    result.z = left.z * mag;

    return result;
}

static r32 v3_mag(v3 left)
{
    r32 result;
    result = sqrtf(left.x * left.x + left.y * left.y + left.z * left.z);
    return result;
}

static r32 v3_mag_sq(v3 left)
{
    r32 result;
    result = left.x * left.x + left.y * left.y + left.z * left.z;
    return result;
}

// Vec4 Pre-decs

static v4 v4_add(v4 left, v4 right)
{
    v4 result;

    result.x = left.x + right.x;
    result.y = left.y + right.y;
    result.z = left.z + right.z;
    result.w = left.w + right.w;

    return result;
}

static v4 v4_sub(v4 left, v4 right)
{
    v4 result;

    result.x = left.x - right.x;
    result.y = left.y - right.y;
    result.z = left.z - right.z;
    result.w = left.w - right.w;

    return result;
}

static v4 v4_mul(v4 left, v4 right)
{
    v4 result;

    result.x = left.x * right.x;
    result.y = left.y * right.y;
    result.z = left.z * right.z;
    result.w = left.w * right.w;

    return result;
}

static v4 v4_mulf(v4 left, r32 right)
{
    v4 result;

    result.x = left.x * right;
    result.y = left.y * right;
    result.z = left.z * right;
    result.w = left.w * right;

    return result;
}

static v4 v4_divf(v4 left, r32 right)
{
    assert(right != 0.0f && "Attempted to divide a Vec4 by 0.0f");

    v4 result;

    result.x = left.x / right;
    result.y = left.y / right;
    result.z = left.z / right;
    result.w = left.w / right;

    return result;
}

static r32 v4_dot(v4 left, v4 right)
{
    r32 result = 0.0f;

    result += left.x * right.x;
    result += left.y * right.y;
    result += left.z * right.z;
    result += left.w * right.w;

    return result;
}

static v4 v4_cross(v4 left, v4 right)
{
    v4 result;

    result.xyz = v3_cross(left.xyz, right.xyz);
    result.w = 1.0f;
    
    return result;
}

static v4 v4_norm(v4 left)
{
    v4 result;

    r32 mag = 1.0f / v4_mag(left);
    result.x = left.x * mag;
    result.y = left.y * mag;
    result.z = left.z * mag;
    result.w = left.w * mag;

    return result;
}

static r32 v4_mag(v4 left)
{
    r32 result;
    result = sqrtf(left.x * left.x + left.y * left.y + left.z * left.z + left.w * left.w);
    return result;
}

static r32 v4_mag_sq(v4 left)
{
    r32 result;
    result = left.x * left.x + left.y * left.y + left.z * left.z + left.w * left.w;
    return result;
}

// MAT3 Defs

static m3 m3_diag(r32 d)
{
    v3 c0 = { .x = d,    .y = 0.0f, .z = 0.0f };
    v3 c1 = { .x = 0.0f, .y = d,    .z = 0.0f };
    v3 c2 = { .x = 0.0f, .y = 0.0f, .z = d    };
    
    m3 result;

    result.c0 = c0;
    result.c1 = c1;
    result.c2 = c2;

    return result;
}

// MAT4 Pre-decs

static m4 m4_diag(r32 d)
{
    m4 result;

    v4 c0 = {.x = d,    .y = 0.0f, .z = 0.0f, .w = 0.0f };
    v4 c1 = {.x = 0.0f, .y = d,    .z = 0.0f, .w = 0.0f };
    v4 c2 = {.x = 0.0f, .y = 0.0f, .z = d   , .w = 0.0f };
    v4 c3 = {.x = 0.0f, .y = 0.0f, .z = 0.0f, .w = d    };
    
    result.c0 = c0;
    result.c1 = c1;
    result.c2 = c2;
    result.c3 = c3;

    return result;
}

static m4 m3_to_m4(m3 mat)
{
    m4 result = M4_IDENTITY;

    result.c0.xyz = mat.c0;
    result.c1.xyz = mat.c1;
    result.c2.xyz = mat.c2;

    return result;
}

static m4 m4_mul(m4 left, m4 r)
{    
    m4 result;

    v4 lr0 = { .x = left.p[0][0], .y = left.p[1][0], .z = left.p[2][0], .w = left.p[3][0] };
    v4 lr1 = { .x = left.p[0][1], .y = left.p[1][1], .z = left.p[2][1], .w = left.p[3][1] };
    v4 lr2 = { .x = left.p[0][2], .y = left.p[1][2], .z = left.p[2][2], .w = left.p[3][2] };
    v4 lr3 = { .x = left.p[0][3], .y = left.p[1][3], .z = left.p[2][3], .w = left.p[3][3] };
    
    result.p[0][0] = v4_dot(lr0, r.c0);
    result.p[0][1] = v4_dot(lr1, r.c0);
    result.p[0][2] = v4_dot(lr2, r.c0);
    result.p[0][3] = v4_dot(lr3, r.c0);

    result.p[1][0] = v4_dot(lr0, r.c1);
    result.p[1][1] = v4_dot(lr1, r.c1);
    result.p[1][2] = v4_dot(lr2, r.c1);
    result.p[1][3] = v4_dot(lr3, r.c1);
    
    result.p[2][0] = v4_dot(lr0, r.c2);
    result.p[2][1] = v4_dot(lr1, r.c2);
    result.p[2][2] = v4_dot(lr2, r.c2);
    result.p[2][3] = v4_dot(lr3, r.c2);
    
    result.p[3][0] = v4_dot(lr0, r.c3);
    result.p[3][1] = v4_dot(lr1, r.c3);
    result.p[3][2] = v4_dot(lr2, r.c3);
    result.p[3][3] = v4_dot(lr3, r.c3);

    return result;
}

/* Creates a scaling matrix */
static m4 m4_scale(r32 sx, r32 sy, r32 sz)
{
    m4 result = M4_IDENTITY;
    
    result.p[0][0] = sx;
    result.p[1][1] = sy;
    result.p[2][2] = sz;
    
    return result;
}

/* Creates a translation matrix */
static m4 m4_translate(v3 trans)
{
    m4 result = M4_IDENTITY;
    
    result.p[3][0] = trans.x;
    result.p[3][1] = trans.y;
    result.p[3][2] = trans.z;
    
    return result;
}

/* Creates a look at matrix */
static m4 m4_look_at(v3 eye, v3 center, v3 up)
{
    m4 result = M4_IDENTITY;
    
    v3 f = v3_norm(v3_sub(center, eye));
    v3 s = v3_norm(v3_cross(f, up));
    v3 u = v3_cross(s, f);
    
    result.p[0][0] = s.x;
    result.p[0][1] = u.x;
    result.p[0][2] = -f.x;
    result.p[0][3] = 0.0f;
    
    result.p[1][0] = s.y;
    result.p[1][1] = u.y;
    result.p[1][2] = -f.y;
    result.p[1][3] = 0.0f;
    
    result.p[2][0] = s.z;
    result.p[2][1] = u.z;
    result.p[2][2] = -f.z;
    result.p[2][3] = 0.0f;
    
    result.p[3][0] = -v3_dot(s, eye);
    result.p[3][1] = -v3_dot(u, eye);
    result.p[3][2] = v3_dot(f, eye);
    result.p[3][3] = 1.0f;
    
    return result;
}

/* Creates a perspective projection matrix */
static m4 m4_perspective(r32 fov, r32 ar, r32 near_plane, r32 far_plane)
{
    m4 result = M4_IDENTITY;
    
    r32 cotangent = 1.0f / tanf(fov * ((r32)MM_PI / 360.0f));
    
#if 1
    result.p[0][0] = cotangent / ar;
    result.p[1][1] = cotangent;
    result.p[2][3] = -1.0f;
    result.p[2][2] = (near_plane + far_plane) / (near_plane - far_plane);
    result.p[3][2] = (2.0f * near_plane * far_plane) / (near_plane - far_plane);
    result.p[3][3] = 0.0f;
#else
    result.data[0][0] = cotangent / ar;
    result.data[1][1] = cotangent;
    result.data[3][2] = -1.0f;
    result.data[2][2] = (near_plane + far_plane) / (near_plane - far_plane);
    result.data[2][3] = (2.0f * near_plane * far_plane) / (near_plane - far_plane);
    result.data[3][3] = 0.0f;
#endif
    
    return result;
}

static m4 m4_rotate_x(r32 theta)
{
    r32 rad = degrees_to_radians(theta);

    r32 c = cosf(rad);
    r32 s = sinf(rad);

    m4 result;

    result.p[0][0] = 1.0f;
    result.p[0][1] = 0.0f;
    result.p[0][2] = 0.0f;
    result.p[0][3] = 0.0f;

    result.p[1][0] = 0.0f;
    result.p[1][1] = c;
    result.p[1][2] = s;
    result.p[1][3] = 0.0f;

    result.p[2][0] = 0.0f;
    result.p[2][1] = -s;
    result.p[2][2] = c;
    result.p[2][3] = 0.0f;

    result.p[3][0] = 0.0f;
    result.p[3][1] = 0.0f;
    result.p[3][2] = 0.0f;
    result.p[3][3] = 1.0f;

    return result;
}

static m4 m4_rotate_y(r32 theta)
{
    r32 rad = degrees_to_radians(theta);

    r32 c = cosf(rad);
    r32 s = sinf(rad);

    m4 result;

    result.p[0][0] = c;
    result.p[0][1] = 0.0f;
    result.p[0][2] = -s;
    result.p[0][3] = 0.0f;

    result.p[1][0] = 0.0f;
    result.p[1][1] = 1.0f;
    result.p[1][2] = 0.0f;
    result.p[1][3] = 0.0f;

    result.p[2][0] = s;
    result.p[2][1] = 0.0f;
    result.p[2][2] = c;
    result.p[2][3] = 0.0f;

    result.p[3][0] = 0.0f;
    result.p[3][1] = 0.0f;
    result.p[3][2] = 0.0f;
    result.p[3][3] = 1.0f;

    return result;
}

static m4 m4_rotate_z(r32 theta)
{
    r32 rad = degrees_to_radians(theta);

    r32 c = cosf(rad);
    r32 s = sinf(rad);

    m4 result;

    result.p[0][0] = c;
    result.p[0][1] = s;
    result.p[0][2] = 0.0f;
    result.p[0][3] = 0.0f;

    result.p[1][0] = -s;
    result.p[1][1] = c;
    result.p[1][2] = 0.0f;
    result.p[1][3] = 0.0f;

    result.p[2][0] = 0.0f;
    result.p[2][1] = 0.0f;
    result.p[2][2] = 1.0f;
    result.p[2][3] = 0.0f;

    result.p[3][0] = 0.0f;
    result.p[3][1] = 0.0f;
    result.p[3][2] = 0.0f;
    result.p[3][3] = 1.0f;

    return result;
}

static m4 m4_rotate(r32 theta, v3 axis)
{
    r32 rad = degrees_to_radians(theta);
    axis = v3_norm(axis);

    r32 c = cosf(rad);
    r32 s = sinf(rad);
    r32 d = 1.0f - c;

    r32 x = axis.x * d;
    r32 y = axis.y * d;
    r32 z = axis.z * d;
    r32 axay = x * axis.y;
    r32 axaz = x * axis.z;
    r32 ayaz = y * axis.z;

    m4 result;

    result.p[0][0] = c + x * axis.x;
    result.p[0][1] = axay + s * axis.z;
    result.p[0][2] = axaz - s * axis.y;
    result.p[0][3] = 0.0f;

    result.p[1][0] = axay - s * axis.z;
    result.p[1][1] = c + y * axis.y;
    result.p[1][2] = ayaz + s * axis.x;
    result.p[1][3] = 0.0f;

    result.p[2][0] = axaz + s * axis.y;
    result.p[2][1] = ayaz - s * axis.x;
    result.p[2][2] = c + z * axis.z;
    result.p[2][3] = 0.0f;

    result.p[3][0] = 0.0f;
    result.p[3][1] = 0.0f;
    result.p[3][2] = 0.0f;
    result.p[3][3] = 1.0f;

    return result;
}

static r32 clamp(r32 min, r32 max, r32 val)
{
    return (val < min) ? min : (val > max) ? max : val;
}

static r32 smoothstep(r32 v0, r32 v1, r32 t)
{
    t = clamp(0.0f, 1.0f, (t - v0) / (v1 - v0));
    return t * t * (3 - 2 * t);
}

static r32 smootherstep(r32 v0, r32 v1, r32 t)
{
    t = clamp(0.0f, 1.0f, (t - v0) / (v1 - v0));
    return t * t * t * (t * (t * 6 - 15) + 10);
}

#endif //MAPLE_MATH_IMPLEMENTATION
