#pragma once

#include <array>
#include <cmath>
#include <memory>

// Methods are inlined because I want to include them in headers
namespace math
{

constexpr float PI = 3.14159265358979323846f;

using float4x4 = std::array<std::array<float, 4>, 4>;

/*
template<class T, uint8_t M> struct vec;

template<class T>
struct vec<T,2>
{
  T x, y;
};

template<class T>
struct vec<T, 3>
{
  T x, y, z;
};

template<class T>
struct vec<T, 4>
{
  T x, y, z, w;
  vec<T, 3> xyz() { return {x, y, z}; }
};

typedef vec<float, 2> float2;
*/

struct float2 { float x, y; };
struct float3 { float x, y, z; };
struct float4
{
  union
  {
    struct { float x, y, z, w; };
    struct { float r, g, b, a; };
  };

  float4() { x = 0; y = 0; z = 0; w = 0; }
  float4(float _x, float _y, float _z, float _w) { x = _x; y = _y; z = _z; w = _w; }
  float4(float3 xyz, float _w) { x = xyz.x; y = xyz.y; z = xyz.z; w = _w; }

  float3 xyz() { return {x, y, z}; }
};

struct byte4 { uint8_t r, g, b, a; };
struct uint2 { uint16_t x, y; };

struct MaterialTextures
{
  byte4* Ka_map;
  byte4* Kd_map;
  byte4* Ks_map;
  byte4* Bump_map;
  uint16_t texturesWidth;
  uint16_t texturesHeight;
};

struct MaterialReflectance
{
  float3 Ka;
  float3 Kd;
  float3 Ks;
};

struct Vertex
{
  float3 position;
  float3 normal;
  float3 tangent;
  float3 bitangent;
  float2 uv;
};

struct Triangle
{
  Vertex v1, v2, v3;
};

// Operators
inline float3 operator-(const float3& v1) { return { -v1.x, -v1.y, -v1.z }; }
inline float3 operator-(const float3& v1, const float3 &v2) { return { v1.x - v2.x, v1.y - v2.y, v1.z - v2.z }; }
inline float3 operator+(const float3& v1, const float3 &v2) { return { v1.x + v2.x, v1.y + v2.y, v1.z + v2.z }; }
inline float3 operator+(const float3& v1, float s1) { return { v1.x + s1, v1.y + s1, v1.z + s1 }; }
inline float3 operator*(float s1, const float3 &v1) { return { v1.x * s1, v1.y * s1, v1.z * s1 }; }
inline float3 operator-(const float3& v1, float s1) { return { v1.x - s1, v1.y - s1, v1.z - s1 }; }
inline float3 operator*(const float3& v1, float s1) { return { v1.x * s1, v1.y * s1, v1.z * s1 }; }
inline float3 operator*(const float3& v1, const float3 &v2) { return { v1.x * v2.x, v1.y * v2.y, v1.z * v2.z }; }
inline float3 operator/(const float3& v1, float s1) { return { v1.x / s1, v1.y / s1, v1.z / s1 }; }

inline float4 operator-(const float4& v1, const float4& v2) { return { v1.x - v2.x, v1.y - v2.y, v1.z - v2.z, v1.w - v2.w }; }
inline float4 operator*(const float4& v1, const float4& v2) { return { v1.x * v2.x, v1.y * v2.y, v1.z * v2.z, v1.w * v2.w }; }
inline float4 operator*(const float4& v1, float s1) { return { v1.x * s1, v1.y * s1, v1.z * s1, v1.w * s1 }; }
inline float4 operator+(const float4& v1, const float4& v2) { return { v1.x + v2.x, v1.y + v2.y, v1.z + v2.z, v1.w + v2.w }; }

inline float3& operator+=(float3& v1, const float3& v2)
{
  v1.x = v1.x + v2.x;
  v1.y = v1.y + v2.y;
  v1.z = v1.z + v2.z;

  return v1;
}

inline float2& operator-=(float2& v1, const float2& v2)
{
  v1.x = v1.x - v2.x;
  v1.y = v1.y - v2.y;

  return v1;
}

inline float2 operator+(const float2& v1, const float s1) { return { v1.x + s1, v1.y + s1 }; }
inline float2 operator+(const float2& v1, const float2& v2) { return { v1.x + v2.x, v1.y + v2.y }; }
inline float2 operator-(const float2& v1, const float2& v2) { return { v1.x - v2.x, v1.y - v2.y }; }
inline float2 operator*(float s1, const float2& v1) { return { v1.x * s1, v1.y * s1 }; }
inline float2 operator*(const float2& v1, float s1) { return { v1.x * s1, v1.y * s1 }; }
inline float2 operator*(const float2& v1, const float2& v2) { return { v1.x * v2.x, v1.y * v2.y }; }
inline float2 operator/(const float2& v1, const float s1) { return { v1.x / s1, v1.y / s1 }; }

inline float2& operator*=(float2& v1, float s1)
{
  v1.x *= s1;
  v1.y *= s1;

  return v1;
}

inline float3& operator*=(float3& v1, float s1)
{
  v1.x *= s1;
  v1.y *= s1;
  v1.z *= s1;

  return v1;
}

inline float4& operator*=(float4& v1, float s1)
{
  v1.x *= s1;
  v1.y *= s1;
  v1.z *= s1;
  v1.w *= s1;

  return v1;
}

// Utils methods
inline float toRad(float deg) { return deg * PI / 180.0f; }
inline float dot(const float4& v1, const float4& v2) { return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z + v1.w * v2.w; }
inline float dot(const float3& v1, const float3& v2) { return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z; }
inline float length(const float3& v1) { return sqrt(v1.x * v1.x + v1.y * v1.y + v1.z * v1.z); }
inline float saturate(float s) { return s < 0.0f ? 0.0f : s > 1.0f ? 1.0f : s; }
inline float3 saturate(const float3& v1) { return { saturate(v1.x), saturate(v1.y), saturate(v1.z) }; }
inline float3 reflect(const float3& v1, const float3& v2) { return v1 - 2 * v2 * dot(v1, v2); }
inline float3 cross(const float3& v1, const float3& v2) { return { v1.y * v2.z - v1.z * v2.y, v1.z * v2.x - v1.x * v2.z, v1.x * v2.y - v1.y * v2.x }; }

template<typename T>
constexpr T interpolate(const T& src, const T& dst, float alpha) { return src + (dst - src) * alpha; }

inline float3 normalize(const float3& v1)
{
  float length = sqrt(v1.x * v1.x + v1.y * v1.y + v1.z * v1.z);

  return {v1.x / length, v1.y / length, v1.z / length};
}

inline float3 sample(byte4* texture, uint2 size, float2 uv)
{
  float3 color;

  float intPart;

  float uvX = uv.x > 1.0f ? modf(uv.x, &intPart) : uv.x;
  float uvY = uv.y > 1.0f ? modf(uv.y, &intPart) : uv.y;

  uvX = uv.x < 0.0f ? 1 - modf(-uv.x, &intPart) : uvX;
  uvY = uv.y < 0.0f ? 1 - modf(-uv.y, &intPart) : uvY;

  uint16_t texX = (uint16_t)floor(uvX * (size.x - 1.0f));
  uint16_t texY = (uint16_t)floor((1.0f - uvY) * (size.y - 1.0f));

  byte4 texColor = texture[texY * size.x + texX];

  color = { texColor.r / 255.0f, texColor.g / 255.0f, texColor.b / 255.0f };

  return color;
}

// More opeartors
inline float4x4 operator*(const float4x4& m1, const float4x4& m2)
{
  const float4 row_11 = { m1[0][0], m1[0][1], m1[0][2], m1[0][3] };
  const float4 row_21 = { m1[1][0], m1[1][1], m1[1][2], m1[1][3] };
  const float4 row_31 = { m1[2][0], m1[2][1], m1[2][2], m1[2][3] };
  const float4 row_41 = { m1[3][0], m1[3][1], m1[3][2], m1[3][3] };

  const float4 col_12 = { m2[0][0], m2[1][0], m2[2][0], m2[3][0] };
  const float4 col_22 = { m2[0][1], m2[1][1], m2[2][1], m2[3][1] };
  const float4 col_32 = { m2[0][2], m2[1][2], m2[2][2], m2[3][2] };
  const float4 col_42 = { m2[0][3], m2[1][3], m2[2][3], m2[3][3] };

  float4x4 mul =
  { {
    {{ math::dot(row_11, col_12), math::dot(row_11, col_22), math::dot(row_11, col_32), math::dot(row_11, col_42) }},
    {{ math::dot(row_21, col_12), math::dot(row_21, col_22), math::dot(row_21, col_32), math::dot(row_21, col_42) }},
    {{ math::dot(row_31, col_12), math::dot(row_31, col_22), math::dot(row_31, col_32), math::dot(row_31, col_42) }},
    {{ math::dot(row_41, col_12), math::dot(row_41, col_22), math::dot(row_41, col_32), math::dot(row_41, col_42) }},
  } };

  return mul;
}

inline float4 operator*(const float4x4& m1, const float4& v1)
{
  const float4 row_11 = { m1[0][0], m1[0][1], m1[0][2], m1[0][3] };
  const float4 row_21 = { m1[1][0], m1[1][1], m1[1][2], m1[1][3] };
  const float4 row_31 = { m1[2][0], m1[2][1], m1[2][2], m1[2][3] };
  const float4 row_41 = { m1[3][0], m1[3][1], m1[3][2], m1[3][3] };

  float4 mul =
  {
    math::dot(row_11, v1),
    math::dot(row_21, v1),
    math::dot(row_31, v1),
    math::dot(row_41, v1),
  };

  return mul;
}

}
