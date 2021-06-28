#ifndef JENGINE_UTILS_VECTOR_MATH_H
#define JENGINE_UTILS_VECTOR_MATH_H

/*

User API:

// VECTORS


inline Vec2 MakeVec2(float *ptr);
inline Vec3 MakeVec3(float *ptr);
inline Vec4 MakeVec4(float *ptr);


// Calculates the dot product of two vectors
inline float dot(Vec2 const& left, Vec2 const& right);
inline float dot(Vec3 const& left, Vec3 const& right);
inline float dot(Vec4 const& left, Vec4 const& right);

// Calculates the cross product of two vectors
// cross pruduct of a vect2 is the same as calculating the
// determinate of 2x2 matrix
inline float cross(Vec2 const& left, Vec2 const& right);
inline Vec3 cross(Vec3 const& left, Vec3 const& right);
// NOTE(Dustin), there really isn't an actual representation
// of a vec4 cross product. Therefore, the cross product of the
// xyz components are computed instead, where the w component is
// set to 1.
inline Vec4 cross(Vec4 const& left, Vec4 const& right);

// Finds the magnitude of a vector
inline float mag(Vec2 const& vector);
inline float mag(Vec3 const& vector);
inline float mag(Vec4 const& vector);

// Finds the squared magnitude of a vector
inline float mag_sq(Vec2 const& vector);
inline float mag_sq(Vec3 const& vector);
inline float mag_sq(Vec4 const& vector);

// Normalizs a vector so that its magnitude == 1
inline Vec2 norm(Vec2 vector);
inline Vec3 norm(Vec3 vector);
inline Vec4 norm(Vec4 vector);

// MATRICES

inline Mat3 Mul(Mat3 const& left, Mat3 const& right);
inline Mat4 Mul(Mat4 const& left, Mat4 const& right);
inline float DegreesToRadians(float theta);
inline Mat3 RotateX(float theta);
inline Mat3 RotateY(float theta);
inline Mat3 RotateZ(float theta);
inline Mat3 Rotate(float theta, Vec3 axis);
inline Mat3 Reflect(Vec3 vec3);
inline Mat3 Scale(float sx, float sy, float sz);
inline Mat3 Scale(float s, Vec3 scale);
// skew is the vector representing the direction along which the skew ocurs
// perp is the vector perpendicular to skew along which vectors are measured to
// to determine how to skew.
inline Mat3 Skew(float theta, Vec3 skew, Vec3 perp);
inline Mat4 Translate(Vec3 trans);

// QUATERNIONS

Vec3 GetQuaternionVector(Quaternion const& quat);
Mat3 GetQuaternionRotationMatrix(Quaternion const& q);
// Creates a quaternion from a rotation matrix
Quaternion CreateQuaternionFromRotationMatrix(const Mat3& m);
Quaternion QuaternionMul(Quaternion const& left, Quaternion const& right);
// rotates the vector around the quaternion using the sandwich product
Vec3 Transform(const Vec3& v, const Quaternion& q);
Quaternion norm(const Quaternion& q);

// OTHER USEFUL GRAPHICS FUNCTIONS
Mat4 LookAt(Vec3 right, Vec3 up, Vec3 forward, Vec3 position)
Mat4 PerspectiveProjection(float fov, float aspect_ratio, float near, float far);

*/

inline float DegreesToRadians(float theta);

struct Vec2
{
    union
    {
        struct { float x, y; };
        struct { float r, g; };
        
        float data[2];
    };
    
    inline float operator[](int i)
    {
        return data[i];
    }
    
    Vec2& operator+=(Vec2 const& other)
    {
        x += other.x;
        y += other.y;
        
        return *this;
    }
    
    Vec2& operator-=(Vec2 const& other)
    {
        x -= other.x;
        y -= other.y;
        
        return *this;
    }
    
    Vec2& operator*=(Vec2 const& other)
    {
        x *= other.x;
        y *= other.y;
        
        return *this;
    }
    
    Vec2& operator/=(Vec2 const& other)
    {
        x = (other.x == 0) ? 0 : x / other.x;
        y = (other.y == 0) ? 0 : y / other.y;
        
        return *this;
    }
    
    Vec2& operator/=(float const& other)
    {
        if (other != 0)
        {
            x /= other;
            y /= other;
        }
        
        return *this;
    }
    
    Vec2& operator*=(float const& other)
    {
        x *= other;
        y *= other;
        
        return *this;
    }
};

struct Vec3
{
    union
    {
        struct { float x, y, z;    };
        struct { Vec2 xy; float z; };
        struct { float x; Vec2 yz; };
        
        struct { float r, g, b;    };
        struct { Vec2 rg; float b; };
        struct { float r; Vec2 gb; };
        
        float data[3];
    };
    
    inline float &operator[](const int &i)
    {
        return data[i];
    }
    
    Vec3& operator+=(Vec3 const& other)
    {
        x += other.x;
        y += other.y;
        z += other.z;
        
        return *this;
    }
    
    Vec3& operator-=(Vec3 const& other)
    {
        x -= other.x;
        y -= other.y;
        z -= other.z;
        
        return *this;
    }
    
    Vec3& operator*=(Vec3 const& other)
    {
        x *= other.x;
        y *= other.y;
        z *= other.z;
        
        return *this;
    }
    
    Vec3& operator/=(Vec3 const& other)
    {
        x = (other.x == 0) ? 0 : x / other.x;
        y = (other.y == 0) ? 0 : y / other.y;
        z = (other.z == 0) ? 0 : z / other.z;
        
        return *this;
    }
    
    Vec3& operator/=(float const& other)
    {
        if (other != 0)
        {
            x /= other;
            y /= other;
            z /= other;
        }
        
        return *this;
    }
    
    Vec3& operator*=(float const& other)
    {
        x *= other;
        y *= other;
        z *= other;
        
        return *this;
    }
};

struct Vec4
{
    union
    {
        struct { float x, y, z, w;          };
        struct { Vec2 xy, zw;               };
        struct { float x; Vec2 yz; float w; };
        struct { Vec3 xyz; float w;         };
        struct { float x; Vec3 yzw;         };
        
        struct { float r, g, b, a;          };
        struct { Vec2 rg, ba;               };
        struct { Vec3 rgb; float a;         };
        struct { float r; Vec2 gb; float a; };
        struct { float r; Vec3 gba;         };
        
        float data[4];
    };
    
    inline float &operator[](const int &i)
    {
        return data[i];
    }
    
    Vec4& operator+=(Vec4 const& other)
    {
        x += other.x;
        y += other.y;
        z += other.z;
        w += other.w;
        
        return *this;
    }
    
    Vec4& operator-=(Vec4 const& other)
    {
        x -= other.x;
        y -= other.y;
        z -= other.z;
        w -= other.w;
        
        return *this;
    }
    
    Vec4& operator*=(Vec4 const& other)
    {
        x *= other.x;
        y *= other.y;
        z *= other.z;
        w *= other.w;
        
        return *this;
    }
    
    Vec4& operator/=(Vec4 const& other)
    {
        x = (other.x == 0) ? 0 : x / other.x;
        y = (other.y == 0) ? 0 : y / other.y;
        z = (other.z == 0) ? 0 : z / other.z;
        w = (other.w == 0) ? 0 : w / other.w;
        
        return *this;
    }
    
    Vec4& operator/=(float const& other)
    {
        if (other != 0)
        {
            x /= other;
            y /= other;
            z /= other;
            w /= other;
        }
        
        return *this;
    }
    
    Vec4& operator*=(float const& other)
    {
        x *= other;
        y *= other;
        z *= other;
        w *= other;
        
        return *this;
    }
};

// Column major
struct Mat3
{
    union
    {
        float data[9];
        
        struct
        {
            Vec3 col0;
            Vec3 col1;
            Vec3 col2;
        };
    };
    
    float *operator[](int idx)
    {
        return &data[3*idx];
    }
    
    const float* operator[](int idx) const
    {
        return &data[3*idx];
    }
    
    // Expects the data to be passed in row order
    // and is converted to column order
    Mat3(float a, float b, float c,
         float e, float f, float g,
         float i, float j, float k)
    {
        data[3*0 + 0] = a;
        data[3*0 + 1] = e;
        data[3*0 + 2] = i;
        
        data[3*1 + 0] = b;
        data[3*1 + 1] = f;
        data[3*1 + 2] = j;
        
        data[3*2 + 0] = c;
        data[3*2 + 1] = g;
        data[3*2 + 2] = k;
    }
    
    Mat3(float ptr[3][3])
    {
        data[3*0 + 0] = ptr[0][0];
        data[3*0 + 1] = ptr[0][1];
        data[3*0 + 2] = ptr[0][2];
        
        data[3*1 + 0] = ptr[1][0];
        data[3*1 + 1] = ptr[1][1];
        data[3*1 + 2] = ptr[1][2];
        
        data[3*2 + 0] = ptr[2][0];
        data[3*2 + 1] = ptr[2][1];
        data[3*2 + 2] = ptr[2][2];
    }
    
    Mat3()
    {
        data[3*0 + 0] = 1;
        data[3*0 + 1] = 0;
        data[3*0 + 2] = 0;
        
        data[3*1 + 0] = 0;
        data[3*1 + 1] = 1;
        data[3*1 + 2] = 0;
        
        data[3*2 + 0] = 0;
        data[3*2 + 1] = 0;
        data[3*2 + 2] = 1;
    }
};

struct Mat4
{
    union
    {
        float data[4][4];
        
        struct
        { // useful for matrix math?
            Vec4 col0;
            Vec4 col1;
            Vec4 col2;
            Vec4 col3;
        };
    };
    
    float* operator[](int idx)
    {
        return &data[idx][0];
    }
    
    const float* operator[](int idx) const
    {
        return &data[idx][0];
    }
    
    Mat4(float diagonal = 0.0f)
    {
        data[0][0] = diagonal;
        data[0][1] = 0.0f;
        data[0][2] = 0.0f;
        data[0][3] = 0.0f;
        
        data[1][0] = 0.0f;
        data[1][1] = diagonal;
        data[1][2] = 0.0f;
        data[1][3] = 0.0f;
        
        data[2][0] = 0.0f;
        data[2][1] = 0.0f;
        data[2][2] = diagonal;
        data[2][3] = 0.0f;
        
        data[3][0] = 0.0f;
        data[3][1] = 0.0f;
        data[3][2] = 0.0f;
        data[3][3] = diagonal;
    }
    
    // Expects the data to be passed in row order
    // and is converted to column order
    Mat4(float a, float b, float c, float d,
         float e, float f, float g, float h,
         float i, float j, float k, float l,
         float m, float n, float o, float p)
    {
        data[0][0] = a;
        data[0][1] = e;
        data[0][2] = i;
        data[0][3] = m;
        
        data[1][0] = b;
        data[1][1] = f;
        data[1][2] = j;
        data[1][3] = n;
        
        data[2][0] = c;
        data[2][1] = g;
        data[2][2] = k;
        data[2][3] = o;
        
        data[3][0] = d;
        data[3][1] = h;
        data[3][2] = l;
        data[3][3] = p;
    }
    
    Mat4(Mat3 mat)
    {
        
        data[0][0] = mat[0][0];
        data[0][1] = mat[0][1];
        data[0][2] = mat[0][2];
        data[0][3] = 0.0f;
        
        data[1][0] = mat[1][0];
        data[1][1] = mat[1][1];
        data[1][2] = mat[1][2];
        data[1][3] = 0.0f;
        
        data[2][0] = mat[2][0];
        data[2][1] = mat[2][1];
        data[2][2] = mat[2][2];
        data[2][3] = 0.0f;
        
        data[3][0] = 0.0f;
        data[3][1] = 0.0f;
        data[3][2] = 0.0f;
        data[3][3] = 1.0f;
    }
};

struct Quaternion
{
    union
    {
        struct { float x, y, z, w;  };
        struct { Vec3 xyz; float w; };
    };
    
    /*
    Quaternion(float _x, float _y, float _z, float _w)
    {
        x = _x;
        y = _y;
        z = _z;
        w = _w;
    }
*/
};


//----------------------------------------------------------------------------------------//
// Pre-declarations
//----------------------------------------------------------------------------------------//

// Operator overloads
inline Vec2 operator+(Vec2 left, Vec2 const& right);
inline Vec3 operator+(Vec3 left, Vec3 const& right);
inline Vec4 operator+(Vec4 left, Vec4 const& right);

inline Vec2 operator-(Vec2 left, Vec2 const& right);
inline Vec3 operator-(Vec3 left, Vec3 const& right);
inline Vec4 operator-(Vec4 left, Vec4 const& right);

inline Vec2 operator*(Vec2 left, Vec2 const& right);
inline Vec3 operator*(Vec3 left, Vec3 const& right);
inline Vec4 operator*(Vec4 left, Vec4 const& right);

inline Vec2 operator/(Vec2 left, Vec2 const& right);
inline Vec3 operator/(Vec3 left, Vec3 const& right);
inline Vec4 operator/(Vec4 left, Vec4 const& right);

inline Vec2 operator/(Vec2 left, float const& denominator);
inline Vec3 operator/(Vec3 left, float const& denominator);
inline Vec4 operator/(Vec4 left, float const& denominator);

inline Vec2 operator*(Vec2 left, float const& scalar);
inline Vec3 operator*(Vec3 left, float const& scalar);
inline Vec4 operator*(Vec4 left, float const& scalar);

inline Mat3 operator*(Mat3 const& left, Mat3 const& r);
inline Mat4 operator*(Mat4 const& left, Mat4 const& r);

Quaternion operator*(const Quaternion& l, const Quaternion& r);

// VECTORS

Vec2 MakeVec2(float *ptr);
Vec3 MakeVec3(float *ptr);
Vec4 MakeVec4(float *ptr);

// Calculates the dot product of two vectors
inline float dot(Vec2 const& left, Vec2 const& right);
inline float dot(Vec3 const& left, Vec3 const& right);
inline float dot(Vec4 const& left, Vec4 const& right);

// Calculates the cross product of two vectors
// cross pruduct of a vect2 is the same as calculating the
// determinate of 2x2 matrix
inline float cross(Vec2 const& left, Vec2 const& right);
inline Vec3 cross(Vec3 const& left, Vec3 const& right);
// NOTE(Dustin), there really isn't an actual representation
// of a vec4 cross product. Therefore, the cross product of the
// xyz components are computed instead, where the w component is
// set to 1.
inline Vec4 cross(Vec4 const& left, Vec4 const& right);

// Finds the magnitude of a vector
inline float mag(Vec2 const& vector);
inline float mag(Vec3 const& vector);
inline float mag(Vec4 const& vector);

// Finds the squared magnitude of a vector
inline float mag_sq(Vec2 const& vector);
inline float mag_sq(Vec3 const& vector);
inline float mag_sq(Vec4 const& vector);

// Normalizs a vector so that its magnitude == 1
inline Vec2 norm(Vec2 vector);
inline Vec3 norm(Vec3 vector);
inline Vec4 norm(Vec4 vector);

// MATRICES

inline Mat3 MakeMat3(float *ptr);
Mat4 MakeMat4(float *ptr);

inline Mat3 Mul(Mat3 const& left, Mat3 const& right);
Mat4 Mul(Mat4 const& left, Mat4 const& right);
inline Mat3 RotateX(float theta);
inline Mat3 RotateY(float theta);
inline Mat3 RotateZ(float theta);
inline Mat3 Rotate(float theta, Vec3 axis);
inline Mat3 Reflect(Vec3 vec3);
Mat4 Scale(float sx, float sy, float sz);
inline Mat4 Scale(float s, Vec3 scale);
// skew is the vector representing the direction along which the skew ocurs
// perp is the vector perpendicular to skew along which vectors are measured to
// to determine how to skew.
inline Mat3 Skew(float theta, Vec3 skew, Vec3 perp);

Mat4 Translate(Vec3 trans);

// QUATERNIONS

Quaternion MakeQuaternion(float x, float y, float z, float w);
Quaternion CreateQuaternion(Vec3 axis, float theta);
Vec3 GetQuaternionVector(Quaternion const& quat);
Mat3 GetQuaternionRotationMatrix(Quaternion const& q);
// Creates a quaternion from a rotation matrix
Quaternion CreateQuaternionFromRotationMatrix(const Mat3& m);
Quaternion QuaternionMul(Quaternion const& left, Quaternion const& right);
// rotates the vector around the quaternion using the sandwich product
Vec3 Transform(const Vec3& v, const Quaternion& q);
Quaternion norm(const Quaternion& q);

Quaternion conjugate(const Quaternion& q);

// OTHER USEFUL GRAPHICS FUNCTIONS
Mat4 LookAt(Vec3 eye, Vec3 center, Vec3 up);
Mat4 PerspectiveProjection(float fov, float aspect_ratio, float near, float far);


#endif // JENGINE_UTILS_VECTOR_MATH_H
