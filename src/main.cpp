#define T_PGE_APPLICATION
#include "../engine/tPixelGameEngine.h"
//#include "OBJ_Loader.h"

#include "Math.h"
#include "Pipeline.h"

#include <filesystem>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

// Assimp
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/material.h>
#include <assimp/postprocess.h>
#include <assimp/matrix4x4.h>
#include <assimp/cimport.h>

using namespace std;
using namespace math;
using namespace Assimp;

class Light : public tDX::PixelGameEngine
{
public:
  Light()
  {
    sAppName = "Light";
  }

  bool OnUserCreate() override
  {
    m_modelScene = m_importer.ReadFile("res/wall/wall.obj",
      aiProcess_PreTransformVertices |
      aiProcess_CalcTangentSpace |
      aiProcess_GenSmoothNormals |
      aiProcess_Triangulate |
      aiProcess_FixInfacingNormals |
      aiProcess_FindInvalidData |
      aiProcess_ValidateDataStructure
    );

    m_screenWidth = ScreenWidth();
    m_screenHeight = ScreenHeight();
    m_aspectRatio = (float)m_screenWidth / (float)m_screenHeight;

    m_pipeline = make_unique<Pipeline>();
    //m_loader = make_unique<objl::Loader>();

    string directory = "wall";
    //m_loader->LoadFile(ReturnObjPath(directory));

    m_depthBuffer = new float[m_screenWidth * m_screenHeight];
    m_renderTarget = make_unique<tDX::Sprite>(m_screenWidth, m_screenHeight);

    //m_PSTexturesSprites.resize(m_loader->LoadedMaterials.size());
    m_PSTexturesSprites.resize(m_modelScene->mNumMaterials);

    for (size_t materialID = 0; materialID < m_modelScene->mNumMaterials; materialID++)
    {
      const aiMaterial* mtl = m_modelScene->mMaterials[materialID];
      aiString texPath;
      aiString name = mtl->GetName();

      if (mtl->GetTexture(aiTextureType_AMBIENT, 0, &texPath) == AI_SUCCESS)
        m_PSTexturesSprites[materialID][MapType::Ka_map].LoadFromFile("res/" + directory + "/" + texPath.C_Str());
      if (mtl->GetTexture(aiTextureType_DIFFUSE, 0, &texPath) == AI_SUCCESS)
        m_PSTexturesSprites[materialID][MapType::Kd_map].LoadFromFile("res/" + directory + "/" + texPath.C_Str());
      if (mtl->GetTexture(aiTextureType_SPECULAR, 0, &texPath) == AI_SUCCESS)
        m_PSTexturesSprites[materialID][MapType::Ks_map].LoadFromFile("res/" + directory + "/" + texPath.C_Str());
      if (mtl->GetTexture(aiTextureType_HEIGHT, 0, &texPath) == AI_SUCCESS)
        m_PSTexturesSprites[materialID][MapType::Bump_map].LoadFromFile("res/" + directory + "/" + texPath.C_Str());
    };

    /*
    for (size_t materialID = 0; materialID < m_PSTexturesSprites.size(); materialID++)
    {
      if (m_loader->LoadedMaterials[materialID].map_Ka.size() > 0)
        m_PSTexturesSprites[materialID][MapType::Ka_map].LoadFromFile("res/" + directory + "/" + m_loader->LoadedMaterials[materialID].map_Ka);
      if (m_loader->LoadedMaterials[materialID].map_Kd.size() > 0)
        m_PSTexturesSprites[materialID][MapType::Kd_map].LoadFromFile("res/" + directory + "/" + m_loader->LoadedMaterials[materialID].map_Kd);
      if (m_loader->LoadedMaterials[materialID].map_Ks.size() > 0)
        m_PSTexturesSprites[materialID][MapType::Ks_map].LoadFromFile("res/" + directory + "/" + m_loader->LoadedMaterials[materialID].map_Ks);
      if (m_loader->LoadedMaterials[materialID].map_bump.size() > 0)
        m_PSTexturesSprites[materialID][MapType::Bump_map].LoadFromFile("res/" + directory + "/" + m_loader->LoadedMaterials[materialID].map_bump);
    };
    */

    /* TODO assimp
    if (m_modelScene->mNumMaterials == 0)
    {
      // Create default material
      objl::Material defaultMaterial = {};

      defaultMaterial.name = "#default";
      defaultMaterial.Kd = { 0.8f, 0.5f, 0.2f };

      m_loader->LoadedMaterials.push_back(defaultMaterial);
    }
    */

    //size_t materialsNum = m_loader->LoadedMaterials.size() + 1; // One more for light
    size_t materialsNum = m_modelScene->mNumMaterials; // One more for light

    m_sortedVerticesByMaterial.resize(materialsNum);
    m_sortedIndicesByMaterial.resize(materialsNum);

    //for (auto& mesh : m_loader->LoadedMeshes)
    for (size_t meshID = 0; meshID < m_modelScene->mNumMeshes; meshID++)
    {
      const aiMesh* mesh = m_modelScene->mMeshes[meshID];

      for (size_t vertexID = 0; vertexID < mesh->mNumVertices; vertexID++)
      {
        const aiVector3D& v = mesh->mVertices[vertexID];
        const aiVector3D& n = mesh->mNormals[vertexID];
        const aiVector3D& t = mesh->mTangents[vertexID];
        const aiVector3D& b = mesh->mBitangents[vertexID];
        const aiVector3D& uv = mesh->mTextureCoords[0][vertexID];

        Vertex vertex;
        vertex.position = { v.x, v.y, v.z };
        vertex.normal = { n.x, n.y, n.z };
        vertex.tangent = { t.x, t.y, t.z };
        vertex.bitangent = { b.x, b.y, b.z };
        vertex.uv = { uv.x, uv.y };

        m_sortedVerticesByMaterial[mesh->mMaterialIndex].push_back(vertex);
      }
    }

    /*
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
    */

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
    if (GetKey(tDX::Z).bHeld) { MoveModel({ 0, coeficient, 0 }); }
    if (GetKey(tDX::C).bHeld) { MoveModel({ 0, -coeficient, 0 }); }
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
  void MoveModel(float3 translation) { m_translation += translation; }

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

    //uint8_t materialsNum = m_drawLight ? (uint8_t)m_loader->LoadedMaterials.size() : (uint8_t)m_loader->LoadedMaterials.size() - 1;
    uint8_t materialsNum = m_modelScene->mNumMaterials;

    MaterialReflectance reflectance = {};

    for (size_t materialID = 0; materialID < materialsNum; materialID++)
    {
      m_pipeline->SetIAInput(m_sortedVerticesByMaterial[materialID], m_sortedIndicesByMaterial[materialID]);

      const aiMaterial* mtl = m_modelScene->mMaterials[materialID];
      aiColor4D color;

      if (mtl->Get(AI_MATKEY_COLOR_AMBIENT, color) == AI_SUCCESS)
        reflectance.Ka = { color.r, color.g, color.b };
      if (mtl->Get(AI_MATKEY_COLOR_DIFFUSE, color) == AI_SUCCESS)
        reflectance.Kd = { color.r, color.g, color.b };
      if (mtl->Get(AI_MATKEY_COLOR_SPECULAR, color) == AI_SUCCESS)
        reflectance.Ks = { color.r, color.g, color.b };

      /*
      reflectance.Ka = { m_loader->LoadedMaterials[materialID].Ka.X, m_loader->LoadedMaterials[materialID].Ka.Y, m_loader->LoadedMaterials[materialID].Ka.Z };
      reflectance.Kd = { m_loader->LoadedMaterials[materialID].Kd.X, m_loader->LoadedMaterials[materialID].Kd.Y, m_loader->LoadedMaterials[materialID].Kd.Z };
      reflectance.Ks = { m_loader->LoadedMaterials[materialID].Ks.X, m_loader->LoadedMaterials[materialID].Ks.Y, m_loader->LoadedMaterials[materialID].Ks.Z };
      */

      /*
      if (materialID == materialsNum) // TODO: Light
      {
        m_pipeline->SetVSBuffers(m_mvpLightMatrix, m_viewMatrix, m_modelMatrix);

        m_materialTextures.Ka_map = nullptr;
        m_materialTextures.Kd_map = nullptr;
        m_materialTextures.Ks_map = nullptr;
        m_materialTextures.Bump_map = nullptr;
        m_materialTextures.texturesHeight = 0;
        m_materialTextures.texturesWidth = 0;
      }
      else
      */
      {
        m_materialTextures.Ka_map = (byte4*)m_PSTexturesSprites[materialID][MapType::Ka_map].GetData();
        m_materialTextures.Kd_map = (byte4*)m_PSTexturesSprites[materialID][MapType::Kd_map].GetData();
        m_materialTextures.Ks_map = (byte4*)m_PSTexturesSprites[materialID][MapType::Ks_map].GetData();
        m_materialTextures.Bump_map = (byte4*)m_PSTexturesSprites[materialID][MapType::Bump_map].GetData();
        m_materialTextures.texturesHeight = m_PSTexturesSprites[materialID][MapType::Kd_map].height;
        m_materialTextures.texturesWidth = m_PSTexturesSprites[materialID][MapType::Kd_map].width;
      }

      m_pipeline->SetPSBuffers(
        reflectance,
        &m_materialTextures,
        m_lightTranslation,
        m_eye
      );

      m_pipeline->Draw();
    }
  }

  void PlaceModelToCenter()
  {
    float3 maxCoords = { numeric_limits<float>::min(), numeric_limits<float>::min(), numeric_limits<float>::min() };
    float3 minCoords = { numeric_limits<float>::max(), numeric_limits<float>::max(), numeric_limits<float>::max() };

    for (size_t meshID = 0; meshID < m_modelScene->mNumMeshes; meshID++)
    {
      const aiMesh* mesh = m_modelScene->mMeshes[meshID];

      for (size_t vertexID = 0; vertexID < mesh->mNumVertices; vertexID++)
      {
        const aiVector3D& v = mesh->mVertices[vertexID];

        maxCoords.x = max(maxCoords.x, v.x);
        maxCoords.y = max(maxCoords.y, v.y);
        maxCoords.z = max(maxCoords.z, v.z);

        minCoords.x = min(minCoords.x, v.x);
        minCoords.y = min(minCoords.y, v.y);
        minCoords.z = min(minCoords.z, v.z);
      }
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
    Ka_map,
    Kd_map,
    Ks_map,
    Bump_map
  };

  unique_ptr<tDX::Sprite> m_renderTarget;

  vector<std::unordered_map<MapType, tDX::Sprite>> m_PSTexturesSprites;

  //std::unique_ptr<objl::Loader> m_loader;
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

  Assimp::Importer  m_importer;
  const aiScene* m_modelScene;
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
  Light light;
  if (light.Construct(800, 600, 1, 1))
    light.Start();

  return 0;
}