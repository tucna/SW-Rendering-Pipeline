#define T_PGE_APPLICATION
#include "../engine/tPixelGameEngine.h"

#include <array>
#include <vector>

#include "OBJ_Loader.h"

#define PI 3.14159265358979323846f

using namespace std;

using float4x4 = array<array<float, 4>, 4>;

struct float4 { float x, y, z, w; };
struct float3 { float x, y, z; };
struct float2 { float x, y; };

// Utils methods
float toRad(float deg) { return deg * PI / 180.0f; }
float dot(const float4& v1, const float4& v2) { return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z + v1.w * v2.w; }
float dot(const float3& v1, const float3& v2) { return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z; }

float3 cross(const float3& v1, const float3& v2)
{
  return { v1.y * v2.z - v1.z * v2.y, v1.z * v2.x - v1.x * v2.z, v1.x * v2.y - v1.y * v2.x };
}

float3 normalize(const float3& v1)
{
  float length = sqrt(v1.x * v1.x + v1.y * v1.y + v1.z * v1.z);

  float3 normalized =
  {
    v1.x / length,
    v1.y / length,
    v1.z / length,
  };

  return normalized;
}

// Operator overloading

float3 operator-(const float3 &v1) { return { -v1.x, -v1.y, -v1.z }; }
float3 operator-(const float3 &v1, const float3 &v2)
{
  float3 difference =
  {
    v1.x - v2.x,
    v1.y - v2.y,
    v1.z - v2.z,
  };

  return difference;
}

float4x4 operator*(const float4x4& m1, const float4x4& m2)
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
    {{ dot(row_11, col_12), dot(row_11, col_22), dot(row_11, col_32), dot(row_11, col_42) }},
    {{ dot(row_21, col_12), dot(row_21, col_22), dot(row_21, col_32), dot(row_21, col_42) }},
    {{ dot(row_31, col_12), dot(row_31, col_22), dot(row_31, col_32), dot(row_31, col_42) }},
    {{ dot(row_41, col_12), dot(row_41, col_22), dot(row_41, col_32), dot(row_41, col_42) }},
  } };

  return mul;
}

float4 operator*(const float4x4& m1, const float4& v1)
{
  const float4 row_11 = { m1[0][0], m1[0][1], m1[0][2], m1[0][3] };
  const float4 row_21 = { m1[1][0], m1[1][1], m1[1][2], m1[1][3] };
  const float4 row_31 = { m1[2][0], m1[2][1], m1[2][2], m1[2][3] };
  const float4 row_41 = { m1[3][0], m1[3][1], m1[3][2], m1[3][3] };

  float4 mul =
  {
    dot(row_11, v1),
    dot(row_21, v1),
    dot(row_31, v1),
    dot(row_41, v1),
  };

  return mul;
}

float2& operator-=(float2& v1, const float2& v2)
{
  v1.x = v1.x - v2.x;
  v1.y = v1.y - v2.y;

  return v1;
}

float2 operator+(const float2& v1, const float s1) { return { v1.x + s1, v1.y + s1 }; }
float2 operator+(const float2& v1, const float2& v2) { return { v1.x + v2.x, v1.y + v2.y }; }

class Light : public tDX::PixelGameEngine
{
public:
  Light() :
    m_zBuffer(800 * 600, -100)
  {
    sAppName = "Light";
    loader.LoadFile("res/bunny.obj");

    float3 maxCoords = {-1000,-1000,-1000};
    float3 minCoords = {1000,1000,1000};

    for (const auto& vertex : loader.LoadedVertices)
    {
      maxCoords.x = max(maxCoords.x, vertex.Position.X);
      maxCoords.y = max(maxCoords.y, vertex.Position.Y);
      maxCoords.z = max(maxCoords.z, vertex.Position.Z);

      minCoords.x = min(minCoords.x, vertex.Position.X);
      minCoords.y = min(minCoords.y, vertex.Position.Y);
      minCoords.z = min(minCoords.z, vertex.Position.Z);
    }

    m_cubeTranslationX = -(minCoords.x + maxCoords.x) / 2.0f;
    m_cubeTranslationY = -(minCoords.y + maxCoords.y) / 2.0f;
    m_cubeTranslationZ = -(minCoords.z + maxCoords.z) / 2.0f;
  }

  bool OnUserCreate() override
  {
    return true;
  }

  bool OnUserUpdate(float fElapsedTime) override
  {
    Clear(tDX::BLACK);

    // Keyboard control
    float coeficient = 10.0f * fElapsedTime;

    if (GetKey(tDX::SHIFT).bHeld) { coeficient *= 8; }
    if (GetKey(tDX::W).bHeld) { m_eye.z -= coeficient; }
    if (GetKey(tDX::S).bHeld) { m_eye.z += coeficient; }
    if (GetKey(tDX::E).bHeld) { m_yaw += coeficient * 15; }
    if (GetKey(tDX::Q).bHeld) { m_yaw -= coeficient * 15; }

    // World matrix
    m_translationMatrix =
    {{
      {{ 1, 0, 0, m_cubeTranslationX }},
      {{ 0, 1, 0, m_cubeTranslationY }},
      {{ 0, 0, 1, m_cubeTranslationZ }},
      {{ 0, 0, 0, 1                  }},
    }};

    m_rotationMatrix =
    { {
      {{ cos(toRad(m_yaw))     , 0, -sin(toRad(m_yaw)), 0 }},
      {{ 0                     , 1, 0                 , 0 }},
      {{ sin(toRad(m_yaw))     , 0, cos(toRad(m_yaw)) , 0 }},
      {{ 0                     , 0, 0                 , 1 }},
    } };

    m_modelMatrix = m_rotationMatrix * m_translationMatrix;

    // View matrix
    float3 zaxis = normalize(m_target - m_eye);
    float3 xaxis = normalize(cross(zaxis, m_up));
    float3 yaxis = cross(xaxis, zaxis);

    zaxis = -zaxis;

    m_viewMatrix =
    { {
      {{ xaxis.x, xaxis.y, xaxis.z, -dot(xaxis, m_eye) }},
      {{ yaxis.x, yaxis.y, yaxis.z, -dot(yaxis, m_eye) }},
      {{ zaxis.x, zaxis.y, zaxis.z, -dot(zaxis, m_eye) }},
      {{ 0, 0, 0, 1 }}
    } };

    // Projection
    float fovY = 45.0f;
    float n = 0.1f;
    float f = 100.0f;
    float tan_fovY = tan(toRad(fovY / 2.0f));

    m_projectionMatrix =
    { {
      {{ 1.0f / (m_aspectRatio*tan_fovY), 0              , 0           , 0                        }},
      {{ 0                              , 1.0f / tan_fovY, 0           , 0                          }},
      {{ 0                              , 0              , -(f + n) / (f - n), -(2 * f*n) / (f - n) }},
      {{ 0                              , 0              , -1          , 0                          }}
    } };

    m_mvpMatrix = m_projectionMatrix * m_viewMatrix * m_modelMatrix;

    for (const auto& mesh : loader.LoadedMeshes)
    {
      for (int id = 0; id < mesh.Indices.size(); id += 3)
      {
        // BFC
        float4 vv1 = m_viewMatrix * m_modelMatrix * float4({ mesh.Vertices[mesh.Indices[id + 0]].Position.X, mesh.Vertices[mesh.Indices[id + 0]].Position.Y, mesh.Vertices[mesh.Indices[id + 0]].Position.Z, 1.0f });
        float4 vn1 = m_viewMatrix * m_modelMatrix * float4({ mesh.Vertices[mesh.Indices[id + 0]].Normal.X, mesh.Vertices[mesh.Indices[id + 0]].Normal.Y, mesh.Vertices[mesh.Indices[id + 0]].Normal.Z, 0.0f });

        float dotP = dot(normalize({vn1.x, vn1.y, vn1.z}), normalize({vv1.x, vv1.y, vv1.z}));

        if (dotP > 0.0f)
          continue;

        float4 v1 = m_mvpMatrix * float4({mesh.Vertices[mesh.Indices[id + 0]].Position.X, mesh.Vertices[mesh.Indices[id + 0]].Position.Y, mesh.Vertices[mesh.Indices[id + 0]].Position.Z, 1.0f});
        float4 v2 = m_mvpMatrix * float4({mesh.Vertices[mesh.Indices[id + 1]].Position.X, mesh.Vertices[mesh.Indices[id + 1]].Position.Y, mesh.Vertices[mesh.Indices[id + 1]].Position.Z, 1.0f});
        float4 v3 = m_mvpMatrix * float4({mesh.Vertices[mesh.Indices[id + 2]].Position.X, mesh.Vertices[mesh.Indices[id + 2]].Position.Y, mesh.Vertices[mesh.Indices[id + 2]].Position.Z, 1.0f});
        /*
        v1.w = v1.w == 0.0f ? 1.0f : v1.w;
        v2.w = v2.w == 0.0f ? 1.0f : v2.w;
        v3.w = v3.w == 0.0f ? 1.0f : v3.w;
        */
        float2 screenV1 = {v1.x / v1.w, v1.y / v1.w};
        float2 screenV2 = {v2.x / v2.w, v2.y / v2.w};
        float2 screenV3 = {v3.x / v3.w, v3.y / v3.w};

        // Viewport
        screenV1.x = (screenV1.x + 1.0f) * 800.0f * 0.5f;
        screenV1.y = (1.0f - screenV1.y) * 600.0f * 0.5f;

        screenV2.x = (screenV2.x + 1.0f) * 800.0f * 0.5f;
        screenV2.y = (1.0f - screenV2.y) * 600.0f * 0.5f;

        screenV3.x = (screenV3.x + 1.0f) * 800.0f * 0.5f;
        screenV3.y = (1.0f - screenV3.y) * 600.0f * 0.5f;

        tDX::vf3d v1s = {screenV1.x, screenV1.y, 1.0f / v1.w };
        tDX::vf3d v2s = {screenV2.x, screenV2.y, 1.0f / v2.w };
        tDX::vf3d v3s = {screenV3.x, screenV3.y, 1.0f / v3.w };

        uint8_t r = uint8_t(min(mesh.MeshMaterial.Kd.X * abs(dotP) + 0.1f/*+ mesh.MeshMaterial.Ka.X*/, 1.0f) * 255);
        uint8_t g = uint8_t(min(mesh.MeshMaterial.Kd.Y * abs(dotP) + 0.1f/*+ mesh.MeshMaterial.Ka.Y*/, 1.0f) * 255);
        uint8_t b = uint8_t(min(mesh.MeshMaterial.Kd.Z * abs(dotP) + 0.1f/*+ mesh.MeshMaterial.Ka.Z*/, 1.0f) * 255);

        //FillTriangle(v1s, v2s, v3s, {r,g,b});
        FillTriangleTUCNA(v1s, v2s, v3s, { r,g,b });
      }
    }

    return true;
  }

private:
  constexpr static float m_aspectRatio = 800.0f / 600.0f;

  objl::Loader loader;

  // Look at
  float3 m_eye = { 0, 0, 10 };
  float3 m_target = { 0, 0, -1 };
  float3 m_up = { 0, 1, 0 };

  float m_cubeTranslationX = 0.0f;
  float m_cubeTranslationY = 0.0f;
  float m_cubeTranslationZ = 0.0f;

  float m_yaw = 0;

  //std::array<std::array<float, 600>, 800> m_zBuffer;
  std::vector<float> m_zBuffer;

  float4x4 m_translationMatrix;
  float4x4 m_rotationMatrix;

  float4x4 m_modelMatrix;
  float4x4 m_viewMatrix;
  float4x4 m_projectionMatrix;

  float4x4 m_mvpMatrix;
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
  Light light;
  if (light.Construct(800, 600, 1, 1))
    light.Start();

  return 0;
}