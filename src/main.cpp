#define T_PGE_APPLICATION
#include "../engine/tPixelGameEngine.h"
#include "OBJ_Loader.h"

#include "Math.h"
#include "Pipeline.h"

#include <memory>
#include <string>
#include <vector>

using namespace std;
using namespace math;

class Light : public tDX::PixelGameEngine
{
public:
  Light()
  {
    sAppName = "Light";
  }

  bool OnUserCreate() override
  {
    m_screenWidth = ScreenWidth();
    m_screenHeight = ScreenHeight();
    m_aspectRatio = (float)m_screenWidth / (float)m_screenHeight;

    m_loader = make_unique<objl::Loader>();
    m_loader->LoadFile("res/cornell_box.obj");

    // TODO
    m_depthBuffer = new float[m_screenWidth * m_screenHeight];
    m_renderTarget = make_unique<tDX::Sprite>(m_screenWidth, m_screenHeight);

    m_pipeline = make_unique<Pipeline>();

    size_t bufferSize = m_loader->LoadedMaterials.size() == 0 ? 1 : m_loader->LoadedMaterials.size();

    m_sortedVerticesByMaterial.resize(bufferSize/* + 1*/); // TODO Light material
    m_sortedIndicesByMaterial.resize(bufferSize/* + 1*/); // TODO Light material

    for (auto& mesh : m_loader->LoadedMeshes)
    {
      for (size_t materialID = 0; materialID < bufferSize; materialID++)
      {
        if (bufferSize == 1 || (mesh.MeshMaterial.name == m_loader->LoadedMaterials[materialID].name))
        {
          for (auto& v : mesh.Vertices)
          {
            Vertex vertex;
            vertex.position = { v.Position.X, v.Position.Y, v.Position.Z };
            vertex.normal = { v.Normal.X, v.Normal.Y, v.Normal.Z };
            vertex.uv = { v.TextureCoordinate.X, v.TextureCoordinate.Y };

            m_sortedVerticesByMaterial[materialID].push_back(vertex);
          }

          for (auto& i : mesh.Indices)
            m_sortedIndicesByMaterial[materialID].push_back(i);
        }
      }
    }

    // TODO Light debug triangle shall be nicer
    /*
    objl::Material lightMaterial = {};
    lightMaterial.Ka = { 1.0f, 1.0f, 1.0f };
    lightMaterial.Kd = { 1.0f, 1.0f, 1.0f };

    m_loader->LoadedMaterials.push_back(lightMaterial);

    m_sortedVerticesByMaterial[m_loader->LoadedMaterials.size() - 1].push_back({ { -0.25f,     0, 0 }, { 0, 1, 0 } });
    m_sortedVerticesByMaterial[m_loader->LoadedMaterials.size() - 1].push_back({ {  0.25f, -0.25, 0 }, { 0, 1, 0 } });
    m_sortedVerticesByMaterial[m_loader->LoadedMaterials.size() - 1].push_back({ {  0.25f,  0.25, 0 }, { 0, 1, 0 } });

    m_sortedIndicesByMaterial[m_loader->LoadedMaterials.size() - 1].push_back((uint32_t)m_sortedVerticesByMaterial[0].size() - 3);
    m_sortedIndicesByMaterial[m_loader->LoadedMaterials.size() - 1].push_back((uint32_t)m_sortedVerticesByMaterial[0].size() - 2);
    m_sortedIndicesByMaterial[m_loader->LoadedMaterials.size() - 1].push_back((uint32_t)m_sortedVerticesByMaterial[0].size() - 1);
    */

    PlaceModelToCenter();

    //m_scene = make_unique<Scene>("res/cornell_box.obj", (byte4*)m_renderTarget->GetData(), ScreenWidth(), ScreenHeight());
    //m_scene = make_unique<Scene>("res/nefertiti.obj", (byte4*)m_renderTarget->GetData(), ScreenWidth(), ScreenHeight());

    //m_PSTexture = make_unique<tDX::Sprite>("res/cube.png");
    //m_scene = make_unique<Scene>("res/cube.obj", (byte4*)m_renderTarget->GetData(), ScreenWidth(), ScreenHeight());
    //m_scene->SetPSTexture((byte4*)m_PSTexture->GetData());

    SetDrawTarget(m_renderTarget.get());

    return true;
  }

  bool OnUserUpdate(float fElapsedTime) override
  {
    Clear(tDX::BLACK);

    // Keyboard control
    float coeficient = 5.0f * fElapsedTime;

    if (GetKey(tDX::SHIFT).bHeld) { coeficient *= 8; }
    if (GetKey(tDX::W).bHeld) { MoveCamera({ 0, 0, -coeficient}); }
    if (GetKey(tDX::S).bHeld) { MoveCamera({ 0, 0, coeficient }); }
    if (GetKey(tDX::E).bHeld) { RotateModel({ 0, coeficient * 15, 0 }); }
    if (GetKey(tDX::Q).bHeld) { RotateModel({ 0, -coeficient * 15, 0 }); }
    if (GetKey(tDX::R).bHeld) { RotateModel({ coeficient * 15, 0, 0 }); }
    if (GetKey(tDX::F).bHeld) { RotateModel({ -coeficient * 15, 0, 0 }); }
    if (GetKey(tDX::NP6).bHeld) { MoveLight({ coeficient, 0, 0 }); }
    if (GetKey(tDX::NP4).bHeld) { MoveLight({ -coeficient, 0, 0 }); }
    if (GetKey(tDX::NP5).bHeld) { MoveLight({ 0, -coeficient, 0 }); }
    if (GetKey(tDX::NP8).bHeld) { MoveLight({ 0, coeficient, 0 }); }
    if (GetKey(tDX::NP7).bHeld) { MoveLight({ 0, 0, -coeficient }); }
    if (GetKey(tDX::NP9).bHeld) { MoveLight({ 0, 0, coeficient }); }

    ComposeMatrices();
    Draw();

    return true;
  }

  void MoveCamera(float3 translation) { m_eye += translation; }
  void MoveLight(float3 translation) { m_lightTranslation += translation; }
  void RotateModel(float3 rotation) { m_rotation += rotation; }

  void Draw()
  {
    // Set pipeline
    m_pipeline->SetRSDescriptor(m_screenWidth, m_screenHeight, Pipeline::Culling::CW);
    m_pipeline->SetOMBuffers(m_depthBuffer, (byte4*)m_renderTarget->GetData(), m_screenWidth, m_screenHeight);
    m_pipeline->SetVSBuffers(m_mvpMatrix, m_viewMatrix, m_modelMatrix);

    m_pipeline->ClearDepthBuffer();

    size_t bufferSize = m_loader->LoadedMaterials.size() == 0 ? 1 : m_loader->LoadedMaterials.size();

    for (size_t materialID = 0; materialID < bufferSize; materialID++)
    {
      /* Ignoring debug lights for now
      if (materialID == bufferSize - 1) // Light
      {
        m_pipeline->SetVSBuffers(m_mvpLightMatrix, m_viewMatrix, m_modelMatrix);
      }
      */

      m_pipeline->SetIAInput(m_sortedVerticesByMaterial[materialID], m_sortedIndicesByMaterial[materialID]);

      if (bufferSize > 1)
      {
        m_pipeline->SetPSBuffers(
          { m_loader->LoadedMaterials[materialID].Kd.X, m_loader->LoadedMaterials[materialID].Kd.Y, m_loader->LoadedMaterials[materialID].Kd.Z },
          { m_loader->LoadedMaterials[materialID].Ka.X, m_loader->LoadedMaterials[materialID].Ka.Y, m_loader->LoadedMaterials[materialID].Ka.Z },
          m_lightTranslation,
          m_eye,
          m_PSTexture,
          256
        );
      }
      else
      {
        m_pipeline->SetPSBuffers(
          { 0.8f, 0.5f, 0.2f },
          { 0.1f, 0.1f, 0.1f },
          m_lightTranslation,
          m_eye,
          m_PSTexture,
          256
        );
      }

      m_pipeline->Draw();
    }
  }

  void PlaceModelToCenter()
  {
    float3 maxCoords = { numeric_limits<float>::min(), numeric_limits<float>::min(), numeric_limits<float>::min() };
    float3 minCoords = { numeric_limits<float>::max(), numeric_limits<float>::max(), numeric_limits<float>::max() };

    for (const auto& vertex : m_loader->LoadedVertices)
    {
      maxCoords.x = max(maxCoords.x, vertex.Position.X);
      maxCoords.y = max(maxCoords.y, vertex.Position.Y);
      maxCoords.z = max(maxCoords.z, vertex.Position.Z);

      minCoords.x = min(minCoords.x, vertex.Position.X);
      minCoords.y = min(minCoords.y, vertex.Position.Y);
      minCoords.z = min(minCoords.z, vertex.Position.Z);
    }

    m_translation.x = -(minCoords.x + maxCoords.x) / 2.0f;
    m_translation.y = -(minCoords.y + maxCoords.y) / 2.0f;
    m_translation.z = -(minCoords.z + maxCoords.z) / 2.0f;

    float cameraPosZ = max(max(maxCoords.x, maxCoords.y), maxCoords.z) * 2.0f;

    m_eye.z = maxCoords.z + cameraPosZ;
    //m_lightTranslation = { 0, 0, m_eye.z };
  }

  void ComposeMatrices()
  {
    // Model matrix
    float4x4 m_translationMatrix =
    { {
      {{ 1, 0, 0, m_translation.x }},
      {{ 0, 1, 0, m_translation.y }},
      {{ 0, 0, 1, m_translation.z }},
      {{ 0, 0, 0, 1               }},
    } };

    float4x4 m_rotationMatrixX =
    { {
      {{ 1 , 0                        , 0                        , 0 }},
      {{ 0 , cos(toRad(m_rotation.x)) , sin(toRad(m_rotation.x)) , 0 }},
      {{ 0 , -sin(toRad(m_rotation.x)), cos(toRad(m_rotation.x)) , 0 }},
      {{ 0 , 0                        , 0                        , 1 }},
    } };

    float4x4 m_rotationMatrixY =
    { {
      {{ cos(toRad(m_rotation.y)) , 0, -sin(toRad(m_rotation.y)), 0 }},
      {{ 0                        , 1, 0                        , 0 }},
      {{ sin(toRad(m_rotation.y)) , 0, cos(toRad(m_rotation.y)) , 0 }},
      {{ 0                        , 0, 0                        , 1 }},
    } };

    m_modelMatrix = m_rotationMatrixY * m_rotationMatrixX * m_translationMatrix;

    float4x4 m_modelLightMatrix =
    { {
      {{ 1, 0, 0, m_lightTranslation.x }},
      {{ 0, 1, 0, m_lightTranslation.y }},
      {{ 0, 0, 1, m_lightTranslation.z }},
      {{ 0, 0, 0, 1 }},
    } };

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
      {{ 1.0f / (m_aspectRatio * tan_fovY), 0              , 0                 , 0                    }},
      {{ 0                                , 1.0f / tan_fovY, 0                 , 0                    }},
      {{ 0                                , 0              , -(f + n) / (f - n), -(2 * f*n) / (f - n) }},
      {{ 0                                , 0              , -1                , 0                    }}
    } };

    m_mvpMatrix = m_projectionMatrix * m_viewMatrix * m_modelMatrix;
    m_mvpLightMatrix = m_projectionMatrix * m_viewMatrix * m_modelLightMatrix;
  }

private:
  unique_ptr<tDX::Sprite> m_renderTarget;
  //unique_ptr<tDX::Sprite> m_PSTexture;

  std::unique_ptr<objl::Loader> m_loader;
  std::unique_ptr<Pipeline> m_pipeline;

  // Matrices
  float3 m_translation = { 0, 0, 0 };
  float3 m_rotation = { 0, 0, 0 };

  float4x4 m_modelMatrix;
  float4x4 m_viewMatrix;
  float4x4 m_projectionMatrix;

  float4x4 m_mvpMatrix;

  // Light
  float3 m_lightTranslation = { 0, 0, 0 };
  float4x4 m_mvpLightMatrix;

  // Look at
  float3 m_eye = { 0, 0, 0 };
  float3 m_target = { 0, 0, -1 };
  float3 m_up = { 0, 1, 0 };

  byte4* m_PSTexture;

  std::vector<std::vector<Vertex>> m_sortedVerticesByMaterial;
  std::vector< std::vector<uint32_t>> m_sortedIndicesByMaterial;

  float* m_depthBuffer;
  float m_aspectRatio;

  uint16_t m_screenWidth;
  uint16_t m_screenHeight;
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
  Light light;
  if (light.Construct(800, 600, 1, 1))
    light.Start();

  return 0;
}