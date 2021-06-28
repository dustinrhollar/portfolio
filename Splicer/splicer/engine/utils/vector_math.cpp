
//----------------------------------------------------------------------------------------//

//----------------------------------------------------------------------------------------//
// Vector Implementations
//----------------------------------------------------------------------------------------//

// Vec2 operator overloads
inline Vec2 operator+(Vec2 left, Vec2 const& right)
{
    left += right;
    return left;
}

inline Vec2 operator-(Vec2 left, Vec2 const& right)
{
    left -= right;
    return left;
}

inline Vec2 operator*(Vec2 left, Vec2 const& right)
{
    left -= right;
    return left;
}

inline Vec2 operator/(Vec2 left, Vec2 const& right)
{
    left += right;
    return left;
}

inline Vec2 operator/(Vec2 left, float const& denominator)
{
    left /= denominator;
    return left;
}

inline Vec2 operator*(Vec2 left, float const& scalar)
{
    left *= scalar;
    return left;
}

// Vec3 operator overloads
inline Vec3 operator+(Vec3 left, Vec3 const& right)
{
    left += right;
    return left;
}

inline Vec3 operator-(Vec3 left, Vec3 const& right)
{
    left -= right;
    return left;
}

inline Vec3 operator*(Vec3 left, Vec3 const& right)
{
    left -= right;
    return left;
}

inline Vec3 operator/(Vec3 left, Vec3 const& right)
{
    left += right;
    return left;
}

inline Vec3 operator/(Vec3 left, float const& denominator)
{
    left /= denominator;
    return left;
}

inline Vec3 operator*(Vec3 left, float const& scalar)
{
    left *= scalar;
    return left;
}

// Vec4 operator overloads
inline Vec4 operator+(Vec4 left, Vec4 const& right)
{
    left += right;
    return left;
}

inline Vec4 operator-(Vec4 left, Vec4 const& right)
{
    left -= right;
    return left;
}

inline Vec4 operator*(Vec4 left, Vec4 const& right)
{
    left -= right;
    return left;
}

inline Vec4 operator/(Vec4 left, Vec4 const& right)
{
    left += right;
    return left;
}

inline Vec4 operator/(Vec4 left, float const& denominator)
{
    left /= denominator;
    return left;
}

inline Vec4 operator*(Vec4 left, float const& scalar)
{
    left *= scalar;
    return left;
}


Vec2 MakeVec2(float *ptr)
{
    Vec2 result;
    
    result.x = ptr[0];
    result.y = ptr[1];
    
    return result;
}

Vec3 MakeVec3(float *ptr)
{
    Vec3 result;
    
    result.x = ptr[0];
    result.y = ptr[1];
    result.z = ptr[2];
    
    return result;
}

Vec4 MakeVec4(float *ptr)
{
    Vec4 result;
    
    result.x = ptr[0];
    result.y = ptr[1];
    result.z = ptr[2];
    result.w = ptr[3];
    
    return result;
}


// Calculates the dot product of two vectors
inline float dot(Vec2 const& left, Vec2 const& right)
{
    return (left.x * right.x)
        + (left.y * right.y);
}

inline float dot(Vec3 const& left, Vec3 const& right)
{
    return (left.x * right.x)
        + (left.y * right.y)
        + (left.z * right.z);
}

inline float dot(Vec4 const& left, Vec4 const& right)
{
    return (left.x * right.x)
        + (left.y * right.y)
        + (left.z * right.z)
        + (left.w * right.w);
}

// Calculates the cross product of two vectors
inline float cross(Vec2 const& left, Vec2 const& right)
{
    return (left.x * right.y) - (left.y * right.x);
}

inline Vec3 cross(Vec3 const& left, Vec3 const& right)
{
    Vec3 result = {};
    
    result.x = (left.y * right.z) - (left.z * right.y);
    result.y = (left.z * right.x) - (left.x * right.z);
    result.z = (left.x * right.y) - (left.y * right.x);
    
    return result;
}

inline Vec4 cross(Vec4 const& left, Vec4 const& right)
{
    Vec4 result = {};
    
    result.xyz = cross(left.xyz, right.xyz);
    result.w   = 1;
    
    return result;
}

// Finds the squared magnitude of a vector
inline float mag_sq(Vec2 const& v)
{
    return (v.x * v.x) + (v.y * v.y);
}

inline float mag_sq(Vec3 const& v)
{
    return (v.x * v.x) + (v.y * v.y) + (v.z * v.z);
}

inline float mag_sq(Vec4 const& v)
{
    return (v.x * v.x) + (v.y * v.y) + (v.z * v.z) + (v.w * v.w);
}

// Finds the magnitude of a vector
inline float mag(Vec2 const& v)
{
    return (float)sqrt((v.x * v.x) + (v.y * v.y));
}

inline float mag(Vec3 const& v)
{
    return (float)sqrt((v.x * v.x) + (v.y * v.y) + (v.z * v.z));
}

inline float mag(Vec4 const& v)
{
    return (float)sqrt((v.x * v.x) + (v.y * v.y) + (v.z * v.z) + (v.w * v.w));
}

// Normalizs a vector so that its magnitude == 1
inline Vec2 norm(Vec2 v)
{
    float m = mag(v);
    v /= m;
    return v;
}

inline Vec3 norm(Vec3 v)
{
    float m = mag(v);
    v /= m;
    return v;
}

inline Vec4 norm(Vec4 v)
{
    float m = mag(v);
    v /= m;
    return v;
}

//----------------------------------------------------------------------------------------//


//----------------------------------------------------------------------------------------//
// Matrix  Implementations
//----------------------------------------------------------------------------------------//

inline Mat3 MakeMat3(float *ptr)
{
    Mat3 result = Mat3();
    
    result.col0 = MakeVec3(ptr);
    result.col1 = MakeVec3(ptr + 3);
    result.col2 = MakeVec3(ptr + 6);
    
    return result;
}

Mat4 MakeMat4(float *ptr)
{
    Mat4 result = Mat4();
    
    result.col0 = MakeVec4(ptr);
    result.col1 = MakeVec4(ptr + 4);
    result.col2 = MakeVec4(ptr + 8);
    result.col3 = MakeVec4(ptr + 12);
    
    return result;
}

inline Mat3 operator*(Mat3 const& left, Mat3 const& r)
{
    Vec3 lr0 = { left[0][0], left[1][0], left[2][0] };
    Vec3 lr1 = { left[0][1], left[1][1], left[2][1] };
    Vec3 lr2 = { left[0][2], left[1][2], left[2][2] };
    
    return Mat3(dot(lr0, r.col0), dot(lr0, r.col1), dot(lr0, r.col2),
                dot(lr1, r.col0), dot(lr1, r.col1), dot(lr1, r.col2),
                dot(lr2, r.col0), dot(lr2, r.col1), dot(lr2, r.col2));
}

inline Mat4 operator*(Mat4 const& left, Mat4 const& r)
{
    Vec4 lr0 = { left[0][0], left[1][0], left[2][0], left[3][0] };
    Vec4 lr1 = { left[0][1], left[1][1], left[2][1], left[3][1] };
    Vec4 lr2 = { left[0][2], left[1][2], left[2][2], left[3][2] };
    Vec4 lr3 = { left[0][3], left[1][3], left[2][3], left[3][3] };
    
    return Mat4(dot(lr0, r.col0), dot(lr0, r.col1), dot(lr0, r.col2), dot(lr0, r.col3),
                dot(lr1, r.col0), dot(lr1, r.col1), dot(lr1, r.col2), dot(lr1, r.col3),
                dot(lr2, r.col0), dot(lr2, r.col1), dot(lr2, r.col2), dot(lr2, r.col3),
                dot(lr3, r.col0), dot(lr3, r.col1), dot(lr3, r.col2), dot(lr3, r.col3));
}

inline Mat3 Mul(Mat3 const& left, Mat3 const& right)
{
    return left * right;
}

Mat4 Mul(Mat4 const& left, Mat4 const& right)
{
    return left * right;
}


inline float DegreesToRadians(float theta)
{
    return (float)(theta * (JPI / 180.0f));
}

inline Mat3 RotateX(float theta)
{
    float rad = DegreesToRadians(theta);
    
    float c = (float)cos(rad);
    float s = (float)sin(rad);
    
    return Mat3(1.0f, 0.0f, 0.0f,
                0.0f, c, -s,
                0.0f, s, c);
}

inline Mat3 RotateY(float theta)
{
    float rad = DegreesToRadians(theta);
    
    float c = (float)cos(rad);
    float s = (float)sin(rad);
    
    return Mat3(c, 0.0f, s,
                0.0f, 1.0f, 0.0f,
                -s, 0.0f, c);
}

inline Mat3 RotateZ(float theta)
{
    float rad = DegreesToRadians(theta);
    
    float c = (float)cos(rad);
    float s = (float)sin(rad);
    
    return Mat3(c, -s, 0.0f,
                s, c, 0.0f,
                0.0f, 0.0f, 1.0f);
}

inline Mat3 Rotate(float theta, Vec3 axis)
{
    float rad = DegreesToRadians(theta);
    axis = norm(axis);
    
    float c = cosf(rad);
    float s = sinf(rad);
    float d = 1.0f - c;
    
    float x = axis.x * d;
    float y = axis.y * d;
    float z = axis.z * d;
    float axay = x * axis.y;
    float axaz = x * axis.z;
    float ayaz = y * axis.z;
    
    return Mat3(   c + x * axis.x, axay - s * axis.z, axaz + s * axis.y,
                axay + s * axis.z,    c + y * axis.y, ayaz - s * axis.x,
                axaz - s * axis.y, ayaz + s * axis.x, c + z * axis.z);
}

inline Mat3 Reflect(Vec3 axis)
{
    axis = norm(axis);
    
    float x = axis.x * -2.0f;
    float y = axis.y * -2.0f;
    float z = axis.z * -2.0f;
    
    float axay = x * axis.y;
    float axaz = x * axis.z;
    float ayaz = y * axis.z;
    
    return Mat3(x * axis.x + 1.0f, axay, axaz,
                axay, y * axis.y + 1.0f, ayaz,
                axaz, ayaz, z * axis.z + 1.0f);
}

Mat4 Scale(float sx, float sy, float sz)
{
    Mat4 result = Mat4(1.0f);
    
    result[0][0] = sx;
    result[1][1] = sy;
    result[2][2] = sz;
    
    return result;
}

inline Mat4 Scale(float s, Vec3 scale)
{
    scale = norm(scale);
    
    s -= 1.0f;
    float x = scale.x * s;
    float y = scale.y * s;
    float z = scale.z * s;
    float axay = x * scale.y;
    float axaz = x * scale.z;
    float ayaz = y * scale.z;
    
    Mat4 result = Mat4(1.0f);
    
    result[0][0] = x * scale.x + 1.0f;
    result[1][0] = axay;
    result[2][0] = axaz;
    result[3][0] = 0.0f;
    
    result[0][1] = axay;
    result[1][1] = y * scale.y + 1.0f;
    result[2][1] = ayaz;
    result[3][1] = 0.0f;
    
    result[0][2] = axaz;
    result[1][2] = ayaz;
    result[2][2] = z * scale.z + 1.0f;
    result[3][2] = 0.0f;
    
    result[0][3] = 0.0f;
    result[0][3] = 0.0f;
    result[0][3] = 0.0f;
    result[0][3] = 1.0f;
    
    return result;
    
}

// skew is the vector representing the direction along which the skew ocurs
// perp is the vector perpendicular to skew along which vectors are measured to
// to determine how to skew.
inline Mat3 Skew(float theta, Vec3 skew, Vec3 perp)
{
    float t = DegreesToRadians(theta);
    Vec3 a = norm(skew);
    Vec3 b = norm(perp);
    
    float r = DegreesToRadians(theta);
    r = (float)tan(r);
    float x = a.x * t;
    float y = a.y * t;
    float z = a.z * t;
    
    return Mat3(x * b.x + 1.0f, x * b.y, x * b.z,
                y * b.x, y * b.y + 1.0f, y * b.z,
                z * b.x, z * b.y, z * b.z + 1.0f);
}

Mat4 Translate(Vec3 trans)
{
    Mat4 result = Mat4(1.0f);
    
    result[3][0] = trans.x;
    result[3][1] = trans.y;
    result[3][2] = trans.z;
    
    return result;
}

//----------------------------------------------------------------------------------------//


//----------------------------------------------------------------------------------------//
// Quaternion Implementations
//----------------------------------------------------------------------------------------//

Quaternion MakeQuaternion(float x, float y, float z, float w)
{
    Quaternion result = {};
    
    result.x = x;
    result.y = y;
    result.z = z;
    result.w = w;
    
    return result;
}


// Creates a quaternion for rotation from an axis and angle (degrees)
Quaternion CreateQuaternion(Vec3 axis, r32 theta)
{
    r32 r = DegreesToRadians(theta);
    r32 half_angle = r * 0.5f;
    
    r32 x = axis.x * sinf(half_angle);
    r32 y = axis.y * sinf(half_angle);
    r32 z = axis.z * sinf(half_angle);
    r32 w = cosf(half_angle);
    
    Quaternion result = {};
    
    result.x = x;
    result.y = y;
    result.z = z;
    result.w = w;
    
    return result;
}

Quaternion operator*(const Quaternion& l, const Quaternion& r)
{
    Quaternion result = {};
    
    result.x = l.w * r.x + l.x * r.w + l.y * r.z - l.z * r.y;
    result.y = l.w * r.y - l.x * r.z + l.y * r.w + l.z * r.x;
    result.z = l.w * r.z + l.x * r.y - l.y * r.x + l.z * r.w;
    result.w = l.w * r.w - l.x * r.x - l.y * r.y - l.z * r.z;
    
    return result;
}

Quaternion QuaternionMul(Quaternion const& left, Quaternion const& right)
{
    return left * right;
}


Vec3 GetQuaternionVector(Quaternion const& quat)
{
    return quat.xyz;
}


Quaternion conjugate(const Quaternion& q)
{
    Quaternion result = {};
    
    result.x = -q.x;
    result.y = -q.y;
    result.z = -q.z;
    result.w = q.w;
    
    return result;
}

Quaternion norm(const Quaternion& q)
{
    float d = 1/(float)sqrt(q.x*q.x + q.y*q.y + q.z*q.z + q.w*q.w);
    
    Quaternion result = {};
    
    result.x = q.x*d;
    result.y = q.y*d;
    result.z = q.z*d;
    result.w = q.w*d;
    
    return result;
}

Vec3 Transform(const Vec3& v, const Quaternion& q)
{
    const Vec3& b = q.xyz;
    float b2 = b.x * b.x + b.y * b.y + b.z * b.z;
    return v * (q.w * q.w - b2) +
        b * (dot(v, b) * 2.0f) +
        cross(b, v) * (q.w * 2.0f);
}

Mat3 GetQuaternionRotationMatrix(Quaternion const& quat)
{
    Quaternion q = norm(quat);
    
    float x2 = q.x * q.x;
    float y2 = q.y * q.y;
    float z2 = q.z * q.z;
    float xy = q.x * q.y;
    float xz = q.x * q.z;
    float yz = q.y * q.z;
    float wx = q.w * q.x;
    float wy = q.w * q.y;
    float wz = q.w * q.z;
    
    return Mat3(1.0f - 2.0f * (y2 + z2), 2.0f * (xy - wz), 2.0f * (xz + wy),
                2.0f * (xy + wz), 1.0f - 2.0f * (x2 + z2), 2.0f * (yz - wx),
                2.0f * (xz - wy), 2.0f * (yz + wx), 1.0f - 2.0f * (x2 + y2));
}

// TODO(Dustin): The if-else might need to swap the matrix indexes
Quaternion CreateQuaternionFromRotationMatrix(const Mat3& m)
{
    float x, y, z, w;
    
    float m00 = m[0][0];
    float m11 = m[1][1];
    float m22 = m[2][2];
    float sum = m00 + m11 + m22;
    
    if (sum > 0.0f)
    {
        w = (float)sqrt(sum + 1.0f) * 0.5f;
        float f = 0.25f / w;
        x = (m[2][1] - m[1][2]) * f;
        y = (m[0][2] - m[2][0]) * f;
        z = (m[1][0] - m[0][1]) * f;
    }
    else if ((m00 > m11) && (m00 > m22))
    {
        x = (float)sqrt(m00 - m11 - m22 + 1.0f) * 0.5f;
        float f = 0.25f / x;
        
        y = (m[1][0] + m[0][1]) * f;
        z = (m[0][2] + m[2][0]) * f;
        w = (m[2][1] - m[1][2]) * f;
    }
    else if (m11 > m22)
    {
        y = (float)sqrt(m11 - m00 - m22 + 1.0f) * 0.5f;
        float f = 0.25f / y;
        
        x = (m[1][0] + m[0][1]) * f;
        z = (m[2][1] + m[1][2]) * f;
        w = (m[0][2] - m[2][1]) * f;
    }
    else
    {
        z = (float)sqrt(m22 - m00 - m11 + 1.0f) * 0.5f;
        float f = 0.25f / z;
        
        x = (m[0][2] + m[2][0]) * f;
        y = (m[2][1] + m[1][2]) * f;
        w = (m[1][0] - m[0][1]) * f;
    }
    
    
    Quaternion result = {};
    
    result.x = x;
    result.y = y;
    result.z = z;
    result.w = w;
    
    return result;
}

//----------------------------------------------------------------------------------------//

Mat4 LookAt(Vec3 eye, Vec3 center, Vec3 up)
{
    Mat4 result = Mat4(1.0f);
    
    Vec3 f = norm(center - eye);
    Vec3 s = norm(cross(f, up));
    Vec3 u = cross(s, f);
    
    result[0][0] = s.x;
    result[0][1] = u.x;
    result[0][2] = -f.x;
    result[0][3] = 0.0f;
    
    result[1][0] = s.y;
    result[1][1] = u.y;
    result[1][2] = -f.y;
    result[1][3] = 0.0f;
    
    result[2][0] = s.z;
    result[2][1] = u.z;
    result[2][2] = -f.z;
    result[2][3] = 0.0f;
    
    result[3][0] = -dot(s, eye);
    result[3][1] = -dot(u, eye);
    result[3][2] = dot(f, eye);
    result[3][3] = 1.0f;
    
    return result;
}

Mat4 PerspectiveProjection(float fov, float aspect_ratio, float near_plane, float far_plane)
{
    Mat4 result = Mat4(1.0f);
    
    float cotangent = 1.0f / tanf(fov * ((float)JPI / 360.0f));
    
    result[0][0] = cotangent / aspect_ratio;
    result[1][1] = cotangent;
    result[2][3] = -1.0f;
    result[2][2] = (near_plane + far_plane) / (near_plane - far_plane);
    result[3][2] = (2.0f * near_plane * far_plane) / (near_plane - far_plane);
    result[3][3] = 0.0f;
    
    return result;
}
