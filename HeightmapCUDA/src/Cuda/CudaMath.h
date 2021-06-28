#ifndef _CUDA_MATH_H
#define _CUDA_MATH_H

#include <curand_kernel.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define DegreesToRadian(theta) (theta * (r32)M_PI)/180.0f

union vec2
{
    struct { r32 x, y; };
    r32 p[2];
    
    HOST_DEVICE vec2() {};
};

union vec3
{
    struct { r32 x, y, z; };
    struct { vec2 xy; r32 z; };
    struct { r32 x; vec2 yz; };
    r32 p[3];
    
    HOST_DEVICE vec3() { x = 0.0f; y = 0.0f; z = 0.0f; };
    HOST_DEVICE vec3(r32 _x, r32 _y, r32 _z) : x(_x), y(_y), z(_z) {}
    
    HOST_DEVICE FORCE_INLINE vec3& operator+=(vec3 right) { x += right.x; y += right.y; z += right.z; return *this; }
    HOST_DEVICE FORCE_INLINE vec3& operator-=(vec3 right) { x -= right.x; y -= right.y; z -= right.z; return *this; }
    HOST_DEVICE FORCE_INLINE vec3& operator*=(vec3 right) { x *= right.x; y *= right.y; z *= right.z; return *this; }
    HOST_DEVICE FORCE_INLINE vec3& operator*=(r32  right) { x *= right; y *= right; z *= right;       return *this; }
    HOST_DEVICE FORCE_INLINE vec3& operator/=(r32  right) { x /= right; y /= right; z /= right;       return *this; }
    HOST_DEVICE FORCE_INLINE vec3 operator-() const { return vec3(-x, -y, -z); }
    
    HOST_DEVICE FORCE_INLINE r32 LengthSq() { return x * x + y * y + z * z; }
    HOST_DEVICE FORCE_INLINE r32 Length() { return sqrtf(LengthSq()); }
    HOST_DEVICE FORCE_INLINE vec3 Norm()
    {
        r32 len = 1.0f / Length();
        vec3 result;
        result.x = x * len;
        result.y = y * len;
        result.z = z * len;
        return result;
    }
    
    HOST_DEVICE FORCE_INLINE bool NearZero()
    {
        const r32 s = 1e-8;
        return (fabs(p[0]) < s) && (fabs(p[1]) < s) && (fabs(p[2]) < s);
    }
    
    static vec3 ZERO;
    static vec3 ONE;
};

#define ZEROVEC3 vec3(0,0,0);
#define ONEVEC3  vec3(1,1,1);

typedef vec2 point2;
typedef vec3 point3;

HOST_DEVICE FORCE_INLINE vec3 operator+(vec3 left, vec3 right) 
{ 
    left += right;
    return left;
}

HOST_DEVICE FORCE_INLINE vec3 operator-(vec3 left, vec3 right) 
{ 
    left -= right;
    return left;
}

HOST_DEVICE FORCE_INLINE vec3 operator*(vec3 left, vec3 right) 
{ 
    left *= right;
    return left;
}

HOST_DEVICE FORCE_INLINE vec3 operator*(vec3 left, r32 right) 
{ 
    left *= right;
    return left;
}

HOST_DEVICE FORCE_INLINE vec3 operator*(r32 left, vec3 right) 
{ 
    right *= left;
    return right;
}

HOST_DEVICE FORCE_INLINE vec3 operator/(vec3 left, r32 right) 
{
    assert(right != 0.0f);
    vec3 result;
    result.x = left.x / right;
    result.y = left.y / right;
    result.z = left.z / right;
    return result;
}

HOST_DEVICE FORCE_INLINE r32 Dot(vec3 left, vec3 other) 
{ 
    return left.x * other.x + left.y * other.y + left.z * other.z; 
}

HOST_DEVICE FORCE_INLINE vec3 Cross(vec3 left, vec3 right) 
{ // TODO(Dustin): Not implemented
    vec3 result;
    
    result.x = (left.y * right.z) - (left.z * right.y);
    result.y = (left.z * right.x) - (left.x * right.z);
    result.z = (left.x * right.y) - (left.y * right.x);
    
    return result;
}

HOST_DEVICE FORCE_INLINE vec3 Reflect(vec3 &v, vec3 &n) 
{
    return v - 2.0f * Dot(v, n) * n;
}

DEVICE FORCE_INLINE
vec3 Refract(vec3 &uv, vec3 &n, r32 etai_over_etat)
{
    r32 cos_theta = fminf(Dot(-uv, n), 1.0f);
    vec3 r_out_perp = etai_over_etat * (uv + cos_theta * n);
    vec3 r_out_parallel = -sqrtf(fabsf(1.0f - r_out_perp.LengthSq())) * n;
    return r_out_perp + r_out_parallel;
}

#define RANDOMFLOAT curand_uniform(local_rand_state)

#define RANDOMVEC3 vec3(RANDOMFLOAT, \
RANDOMFLOAT, \
RANDOMFLOAT)

DEVICE FORCE_INLINE
vec3 RandomInUnitSphere(curandState *local_rand_state)
{
    vec3 p = vec3();
    do
    {
        p = 2.0f * RANDOMVEC3 - ONEVEC3;
    } while (p.LengthSq() >= 1.0f);
    return p;
}

DEVICE FORCE_INLINE
vec3 RandomUnitVector(curandState *local_rand_state)
{
    return RandomInUnitSphere(local_rand_state).Norm();
}

DEVICE FORCE_INLINE
vec3 RandomInHemisphere(curandState *local_rand_state, vec3 &normal)
{
    vec3 in_unit_sphere = RandomInUnitSphere(local_rand_state);
    if (Dot(in_unit_sphere, normal) > 0.0f) return in_unit_sphere;
    else                                    return -in_unit_sphere;
}

DEVICE FORCE_INLINE
vec3 RandomInUnitDisk(curandState *local_rand_state)
{
    vec3 p;
    do
    {
        p = 2.0f * vec3(RANDOMFLOAT, RANDOMFLOAT, 0.0f) - vec3(1,1,0);
        if (p.LengthSq() < 1.0f) return p;;
    } while(Dot(p, p) >= 1.0f);
    return p;
}

#endif //_CUDA_MATH_H
