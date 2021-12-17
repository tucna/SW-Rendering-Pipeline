#include "OBJ_Loader.h"
#include "Scene.h"

#include "../engine/tPixelGameEngine.h"

using namespace std;
using namespace math;

Scene::Scene(const std::string& pathToModel, uint8_t* renderTarget, tDX::PixelGameEngine* engine)
{
  m_renderTarget = renderTarget;
  m_engine = engine;

  m_loader = make_unique<objl::Loader>();
  m_loader->LoadFile(pathToModel);

  // TODO
  m_depthBuffer = new float[800 * 600];

  m_pipeline = make_unique<Pipeline>(m_engine);

  size_t bufferSize = m_loader->LoadedMaterials.size() == 0 ? 1 : m_loader->LoadedMaterials.size();

  m_sortedVerticesByMaterial.resize(bufferSize);
  m_sortedIndicesByMaterial.resize(bufferSize);

  for (auto& mesh : m_loader->LoadedMeshes)
  {
    for (size_t materialID = 0; materialID < bufferSize; materialID++)
    {
      if (bufferSize == 1 || (mesh.MeshMaterial.name == m_loader->LoadedMaterials[materialID].name))
      {
        for (auto& v : mesh.Vertices)
        {
          Vertex vertex;
          vertex.position = {v.Position.X, v.Position.Y, v.Position.Z};
          vertex.normal = { v.Normal.X, v.Normal.Y, v.Normal.Z };

          m_sortedVerticesByMaterial[materialID].push_back(vertex);
        }

        for (auto& i : mesh.Indices)
          m_sortedIndicesByMaterial[materialID].push_back(i);
      }
    }
  }

  PlaceModelToCenter();
}

Scene::~Scene()
{
}

void Scene::MoveCamera(float3 translation)
{
  m_eye += translation;
}

void Scene::RotateModel(float3 rotation)
{
  m_rotation += rotation;
}

void Scene::Draw()
{
  // Set pipeline
  m_pipeline->SetRSDescriptor(800, 600, Pipeline::Culling::CW);
  m_pipeline->SetOMBuffers(m_depthBuffer, m_renderTarget);
  m_pipeline->SetVSBuffers(m_mvpMatrix, m_viewMatrix, m_modelMatrix);

  m_pipeline->ClearDepthBuffer();

  size_t bufferSize = m_loader->LoadedMaterials.size() == 0 ? 1 : m_loader->LoadedMaterials.size();

  for (size_t materialID = 0; materialID < bufferSize; materialID++)
  {
    m_pipeline->SetIAInput(m_sortedVerticesByMaterial[materialID], m_sortedIndicesByMaterial[materialID]);

    if (bufferSize > 1)
    {
      m_pipeline->SetPSBuffers(
        { m_loader->LoadedMaterials[materialID].Kd.X, m_loader->LoadedMaterials[materialID].Kd.Y, m_loader->LoadedMaterials[materialID].Kd.Z },
        { m_loader->LoadedMaterials[materialID].Ka.X, m_loader->LoadedMaterials[materialID].Ka.Y, m_loader->LoadedMaterials[materialID].Ka.Z }
      );
    }
    else
    {
      m_pipeline->SetPSBuffers(
        { 0.8f, 0.8f, 0.8f },
        { 0.1f, 0.1f, 0.1f }
      );
    }

    m_pipeline->Draw();
  }
}

vector<Triangle> Scene::GenerateTrianglesToDraw()
{
  //ComposeMatrices();

  vector<Triangle> triangles;
  /*
  for (const auto& mesh : m_loader->LoadedMeshes)
  {
    for (int id = 0; id < mesh.Indices.size(); id += 3)
    {
      // BFC
      float4 vv1 = m_viewMatrix * m_modelMatrix * float4({ mesh.Vertices[mesh.Indices[id + 0]].Position.X, mesh.Vertices[mesh.Indices[id + 0]].Position.Y, mesh.Vertices[mesh.Indices[id + 0]].Position.Z, 1.0f });
      float4 vn1 = m_viewMatrix * m_modelMatrix * float4({ mesh.Vertices[mesh.Indices[id + 0]].Normal.X, mesh.Vertices[mesh.Indices[id + 0]].Normal.Y, mesh.Vertices[mesh.Indices[id + 0]].Normal.Z, 0.0f });

      float dotP = dot(normalize({ vn1.x, vn1.y, vn1.z }), normalize({ vv1.x, vv1.y, vv1.z }));

      if (dotP > 0.0f)
        continue;

      float4 v1 = m_mvpMatrix * float4({ mesh.Vertices[mesh.Indices[id + 0]].Position.X, mesh.Vertices[mesh.Indices[id + 0]].Position.Y, mesh.Vertices[mesh.Indices[id + 0]].Position.Z, 1.0f });
      float4 v2 = m_mvpMatrix * float4({ mesh.Vertices[mesh.Indices[id + 1]].Position.X, mesh.Vertices[mesh.Indices[id + 1]].Position.Y, mesh.Vertices[mesh.Indices[id + 1]].Position.Z, 1.0f });
      float4 v3 = m_mvpMatrix * float4({ mesh.Vertices[mesh.Indices[id + 2]].Position.X, mesh.Vertices[mesh.Indices[id + 2]].Position.Y, mesh.Vertices[mesh.Indices[id + 2]].Position.Z, 1.0f });

      if (v1.w <= 0 || v2.w <= 0 || v3.w <= 0)
        continue;

      float2 screenV1 = { v1.x / v1.w, v1.y / v1.w };
      float2 screenV2 = { v2.x / v2.w, v2.y / v2.w };
      float2 screenV3 = { v3.x / v3.w, v3.y / v3.w };

      // Viewport
      screenV1.x = (screenV1.x + 1.0f) * 800.0f * 0.5f;
      screenV1.y = (1.0f - screenV1.y) * 600.0f * 0.5f;

      screenV2.x = (screenV2.x + 1.0f) * 800.0f * 0.5f;
      screenV2.y = (1.0f - screenV2.y) * 600.0f * 0.5f;

      screenV3.x = (screenV3.x + 1.0f) * 800.0f * 0.5f;
      screenV3.y = (1.0f - screenV3.y) * 600.0f * 0.5f;

      float3 v1s = { screenV1.x, screenV1.y, 1.0f / v1.w };
      float3 v2s = { screenV2.x, screenV2.y, 1.0f / v2.w };
      float3 v3s = { screenV3.x, screenV3.y, 1.0f / v3.w };
      */

      //uint8_t r = uint8_t(min(mesh.MeshMaterial.Kd.X * abs(dotP) + 0.1f/*+ mesh.MeshMaterial.Ka.X*/, 1.0f) * 255);
      //uint8_t g = uint8_t(min(mesh.MeshMaterial.Kd.Y * abs(dotP) + 0.1f/*+ mesh.MeshMaterial.Ka.Y*/, 1.0f) * 255);
      //uint8_t b = uint8_t(min(mesh.MeshMaterial.Kd.Z * abs(dotP) + 0.1f/*+ mesh.MeshMaterial.Ka.Z*/, 1.0f) * 255);
  /*
      triangles.push_back({v1s, v2s, v3s, r, g, b});
    }
  }
  */

  return triangles;
}

void Scene::PlaceModelToCenter()
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

  m_eye.z = maxCoords.z + 10.0f;
}

void Scene::ComposeMatrices()
{
  // Model matrix
  float4x4 m_translationMatrix =
  { {
    {{ 1, 0, 0, m_translation.x }},
    {{ 0, 1, 0, m_translation.y }},
    {{ 0, 0, 1, m_translation.z }},
    {{ 0, 0, 0, 1               }},
  } };

  float4x4 m_rotationMatrix =
  { {
    {{ cos(toRad(m_rotation.y)) , 0, -sin(toRad(m_rotation.y)), 0 }},
    {{ 0                        , 1, 0                        , 0 }},
    {{ sin(toRad(m_rotation.y)) , 0, cos(toRad(m_rotation.y)) , 0 }},
    {{ 0                        , 0, 0                        , 1 }},
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
    {{ 1.0f / (m_aspectRatio * tan_fovY), 0              , 0                 , 0                    }},
    {{ 0                                , 1.0f / tan_fovY, 0                 , 0                    }},
    {{ 0                                , 0              , -(f + n) / (f - n), -(2 * f*n) / (f - n) }},
    {{ 0                                , 0              , -1                , 0                    }}
  } };

  m_mvpMatrix = m_projectionMatrix * m_viewMatrix * m_modelMatrix;
}
