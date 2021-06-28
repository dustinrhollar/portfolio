#ifndef _FIXED_POINT_H
#define _FIXED_POINT_H

// Multiiplication and other operation
// http://www.sunshine2k.de/articles/coding/fp/sunfp.html

// TODO(Dustin): 32 bit implementation
union FixedPoint32
{
    static const u8 FIXED_POINT_INT_BITS = 16;
    static const u8 FIXED_POINT_DEC_BITS = 32 - FIXED_POINT_INT_BITS;
    static const u32 ONE = 1 << FIXED_POINT_DEC_BITS;
    
    struct
    {
        i32 dec:FIXED_POINT_DEC_BITS;
        i32 inp:FIXED_POINT_INT_BITS; // integet part
    } _parts;
    i32 val;
    
    
    inline FixedPoint32() = default;
    inline explicit FixedPoint32(r32 v) 
    {
        // Source: https://www.cs.cmu.edu/~rbd/papers/cmj-float-to-int.html
        val = (i32)(v * ONE + (((i32) (v + 32768.5)) - 32768));
    }
    
    inline explicit FixedPoint32(i32 v) 
    {
        // NOTE(Dustin): Can I just set v equal to the inp part?
        // will there be an overflow of bits? Will I care?
        val = v << FIXED_POINT_DEC_BITS;
    }
};

union FixedPoint64
{
    static const u32 FIXED_POINT_INT_BITS = 32;
    static const u32 FIXED_POINT_DEC_BITS = 64 - FIXED_POINT_INT_BITS;
    static const u64 ONE = (u64)1 << FIXED_POINT_DEC_BITS;
    
    // NOTE(Dustin): This struct layout assumed a little endian
    // machine.
    struct
    {
        i64 dec:FIXED_POINT_DEC_BITS;
        i64 inp:FIXED_POINT_INT_BITS; // integer part
    } _parts;
    i64 val;
    
    inline FixedPoint64() = default;
    inline explicit FixedPoint64(r32 v) 
    {
        // Source: https://www.cs.cmu.edu/~rbd/papers/cmj-float-to-int.html
        val = (i64)(v * ONE + (((i32) (v + 32768.5)) - 32768));
    }
    
    inline explicit FixedPoint64(i64 v) 
    {
        // NOTE(Dustin): Can I just set v equla to the inp part?
        // will there be an overflow of bits? Will I care?
        val = v << FIXED_POINT_DEC_BITS;
    }
    
    inline FixedPoint64 operator+=(FixedPoint64 b)
    {
        val += b.val;
        return *this;
    }
    
    inline FixedPoint64 operator-=(FixedPoint64 b)
    {
        val -= b.val;
        return *this;
    }
    
    explicit operator float() const { return (r32)(val * (1.0f / (r32)((i64)1 << FIXED_POINT_DEC_BITS))); }
    explicit operator i64() const 
    { 
        i64 result = val >> FIXED_POINT_DEC_BITS;
        return (result < 0 && _parts.dec != 0) ? result + 1 : result; 
    }
    
    explicit operator i32() const 
    { 
        i64 result = val >> FIXED_POINT_DEC_BITS;
        result = (result < 0 && _parts.dec != 0) ? result + 1 : result;
        return (i32)result;
    }
};

typedef FixedPoint64 f64;
typedef FixedPoint32 f32;

#define f32() FixedPoint32()
#define f32f(x) FixedPoint32((r32)(x))
#define f32i(x) FixedPoint32((i32)(x))

#define f64() FixedPoint64()
#define f64f(x) FixedPoint64((r32)(x))
#define f64i(x) FixedPoint64((i64)(x))

inline f32 operator+(f32 left, f32 right);
inline f32 operator-(f32 left, f32 right);

inline f64 operator+(f64 left, f64 right);
inline f64 operator-(f64 left, f64 right);

inline bool operator==(f64 left, f64 right);
inline bool operator!=(f64 left, f64 right);
inline bool operator>=(f64 left, f64 right);
inline bool operator<=(f64 left, f64 right);
inline bool operator>(f64 left, f64 right);
inline bool operator<(f64 left, f64 right);

inline i32 fast_sign(f32 v);
inline i32 fast_sign(f64 v);

inline f32 fast_clamp(f32 min, f32 max, f32 val);
inline f64 fast_clamp(f64 min, f64 max, f64 val);

inline f64 fast_min(f64 min, f64 max);
inline f64 fast_max(f64 min, f64 max);

inline f64 fast_abs(f64 v);

#endif //_FIXED_POINT_H

#if defined(MAPLE_FIXED_POINT_IMPLEMENTATION)

inline f32 operator+(f32 left, f32 right)
{
    f32 result;
    result.val = left.val + right.val;
    return result;
}

inline f32 operator-(f32 left, f32 right)
{
    f32 result;
    result.val = left.val - right.val;
    return result;
}

inline f32 fast_clamp(f32 min, f32 max, f32 val)
{
    return f32();
}

inline i32 fast_sign(f32 v)
{
    return (((unsigned)-v.val >> 31) - ((unsigned)v.val >> 31));
}

inline bool operator==(f64 left, f64 right) {return left.val == right.val;}
inline bool operator!=(f64 left, f64 right) {return left.val != right.val;}
inline bool operator>=(f64 left, f64 right) {return left.val >= right.val;}
inline bool operator<=(f64 left, f64 right) {return left.val <= right.val;}
inline bool operator>(f64 left, f64 right)  {return left.val > right.val;}
inline bool operator<(f64 left, f64 right)  {return left.val < right.val;}

inline f64 operator+(f64 left, f64 right)
{
    f64 result;
    result.val = left.val + right.val;
    return result;
}

inline f64 operator-(f64 left, f64 right)
{
    f64 result;
    result.val = left.val - right.val;
    return result;
}

inline i32 fast_sign(f64 v)
{
    return (((u64)-v.val >> 63) - ((u64)v.val >> 63));
}

inline f64 fast_min(f64 min, f64 max)
{
    f64 sub = min - max;
    
    f64 result;
    result.val = max.val + ((sub.val) & (i64)sex((i32)sub._parts.inp));
    return result; 
}

inline f64 fast_max(f64 min, f64 max)
{
    f64 sub = max - min;
    
    f64 result;
    result.val = min.val + ((sub.val) & ~((i64)sex((i32)sub._parts.inp)));
    return result; 
}

inline f64 fast_abs(f64 v)
{
    f64 result;
    result.val = (v.val ^ (i64)sex((i32)v.val)) - (i64)sex((i32)v.val);
    return result;
}

inline f64 fast_clamp(f64 min, f64 max, f64 val)
{
    return fast_min(fast_max(val, min), max);
}

#endif