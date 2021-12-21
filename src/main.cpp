#define T_PGE_APPLICATION
#include "../engine/tPixelGameEngine.h"
#include "OBJ_Loader.h"

#include "Math.h"
#include "Pipeline.h"

#include <filesystem>
#include <memory>
#include <string>
#include <unordered_map>
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

    m_pipeline = make_unique<Pipeline>();
    m_loader = make_unique<objl::Loader>();

    string directory = "cornell";
    m_loader->LoadFile(ReturnObjPath(directory));

    m_depthBuffer = new float[m_screenWidth * m_screenHeight];
    m_renderTarget = make_unique<tDX::Sprite>(m_screenWidth, m_screenHeight);

    m_PSTexturesSprites.resize(m_loader->LoadedMaterials.size());

    for (size_t materialID = 0; materialID < m_PSTexturesSprites.size(); materialID++)
    {
      if (m_loader->LoadedMaterials[materialID].map_Kd.size() > 0)
      {
        m_PSTexturesSprites[materialID][MapType::KA_MAP].LoadFromFile("res/" + directory + "/" + m_loader->LoadedMaterials[materialID].map_Ka);
        m_PSTexturesSprites[materialID][MapType::KD_MAP].LoadFromFile("res/" + directory + "/" + m_loader->LoadedMaterials[materialID].map_Kd);
        m_PSTexturesSprites[materialID][MapType::KS_MAP].LoadFromFile("res/" + directory + "/" + m_loader->LoadedMaterials[materialID].map_Ks);
      }
    };

    if (m_loader->LoadedMaterials.empty())
    {
      // Create default material
      objl::Material defaultMaterial = {};

      defaultMaterial.name = "#default";
      defaultMaterial.Kd = { 0.8f, 0.5f, 0.2f };

      m_loader->LoadedMaterials.push_back(defaultMaterial);
    }

    size_t materialsNum = m_loader->LoadedMaterials.size() + 1; // One more for light

    m_sortedVerticesByMaterial.resize(materialsNum);
    m_sortedIndicesByMaterial.resize(materialsNum);

    for (auto& mesh : m_loader->LoadedMeshes)
    {
      if (mesh.MeshMaterial.name.empty())
        mesh.MeshMaterial.name = "#default";

      for (size_t materialID = 0; materialID < materialsNum - 1; materialID++) // We can skip last material - light
      {
        if (mesh.MeshMaterial.name == m_loader->LoadedMaterials[materialID].name)
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

    objl::Material lightMaterial = {};
    lightMaterial.name = "light";
    lightMaterial.Ka = { 1.0f, 1.0f, 1.0f };
    lightMaterial.Kd = { 1.0f, 1.0f, 1.0f };

    m_loader->LoadedMaterials.push_back(lightMaterial);

    uint8_t lightMaterialID = (uint8_t)m_loader->LoadedMaterials.size() - 1;

    m_sortedVerticesByMaterial[lightMaterialID].push_back({ { -0.25f,     0, 0 }, { 0, 1, 0 } });
    m_sortedVerticesByMaterial[lightMaterialID].push_back({ {  0.25f, -0.25, 0 }, { 0, 1, 0 } });
    m_sortedVerticesByMaterial[lightMaterialID].push_back({ {  0.25f,  0.25, 0 }, { 0, 1, 0 } });

    m_sortedIndicesByMaterial[lightMaterialID].push_back((uint32_t)m_sortedVerticesByMaterial[0].size() - 3);
    m_sortedIndicesByMaterial[lightMaterialID].push_back((uint32_t)m_sortedVerticesByMaterial[0].size() - 2);
    m_sortedIndicesByMaterial[lightMaterialID].push_back((uint32_t)m_sortedVerticesByMaterial[0].size() - 1);

    PlaceModelToCenter();

    // Engine setup
    SetDrawTarget(m_renderTarget.get());

    return true;
  }

  bool OnUserUpdate(float fElapsedTime) override
  {
    Clear(tDX::BLACK);

    // Keyboard control
    float coeficient = 5.0f * fElapsedTime;

    if (GetKey(tDX::L).bHeld) { m_drawLight = !m_drawLight; }
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

  string ReturnObjPath(const string& directory)
  {
    string path = "res/" + directory + "/";

    for (const auto& file : filesystem::directory_iterator(path))
    {
      if (file.path().extension() == ".obj")
        return file.path().string();
    }

    return "";
  }

  void Draw()
  {
    // Set pipeline
    m_pipeline->SetRSDescriptor(m_screenWidth, m_screenHeight, Pipeline::Culling::CW);
    m_pipeline->SetOMBuffers(m_depthBuffer, (byte4*)m_renderTarget->GetData(), m_screenWidth, m_screenHeight);
    m_pipeline->SetVSBuffers(m_mvpMatrix, m_viewMatrix, m_modelMatrix);

    m_pipeline->ClearDepthBuffer();

    uint8_t materialsNum = m_drawLight ? (uint8_t)m_loader->LoadedMaterials.size() : (uint8_t)m_loader->LoadedMaterials.size() - 1;

    for (size_t materialID = 0; materialID < materialsNum; materialID++)
    {
      m_pipeline->SetIAInput(m_sortedVerticesByMaterial[materialID], m_sortedIndicesByMaterial[materialID]);

      if (materialID == m_loader->LoadedMaterials.size() - 1) // Light
      {
        m_pipeline->SetVSBuffers(m_mvpLightMatrix, m_viewMatrix, m_modelMatrix);

        m_materialTextures.Kd_map = nullptr;
        m_materialTextures.texturesHeight = 0;
        m_materialTextures.texturesWidth = 0;
      }
      else
      {
        m_materialTextures.Kd_map = (byte4*)m_PSTexturesSprites[materialID][MapType::KD_MAP].GetData();
        m_materialTextures.texturesHeight = m_PSTexturesSprites[materialID][MapType::KD_MAP].height;
        m_materialTextures.texturesWidth = m_PSTexturesSprites[materialID][MapType::KD_MAP].width;
      }

      m_pipeline->SetPSBuffers(
        { m_loader->LoadedMaterials[materialID].Kd.X, m_loader->LoadedMaterials[materialID].Kd.Y, m_loader->LoadedMaterials[materialID].Kd.Z },
        { m_loader->LoadedMaterials[materialID].Ka.X, m_loader->LoadedMaterials[materialID].Ka.Y, m_loader->LoadedMaterials[materialID].Ka.Z },
        m_lightTranslation,
        m_eye,
        &m_materialTextures
      );

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
  enum class MapType
  {
    KA_MAP,
    KD_MAP,
    KS_MAP
  };

  unique_ptr<tDX::Sprite> m_renderTarget;

  vector<std::unordered_map<MapType, tDX::Sprite>> m_PSTexturesSprites;

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

  std::vector<std::vector<Vertex>> m_sortedVerticesByMaterial;
  std::vector<std::vector<uint32_t>> m_sortedIndicesByMaterial;

  float* m_depthBuffer;
  float m_aspectRatio;

  uint16_t m_screenWidth;
  uint16_t m_screenHeight;

  MaterialTextures m_materialTextures;

  bool m_drawLight = false;
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
  Light light;
  if (light.Construct(800, 600, 1, 1))
    light.Start();

  return 0;
}