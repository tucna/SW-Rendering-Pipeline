#include "OBJ_Loader.h"
#include "Scene.h"

using namespace std;
using namespace math;

Scene::Scene(const std::string& pathToModel, byte4* renderTarget, uint16_t screenWidth, uint16_t screenHeight) :
  m_screenWidth(screenWidth),
  m_screenHeight(screenHeight),
  m_renderTarget(renderTarget)
{
  m_aspectRatio = (float)screenWidth / (float)screenHeight;

  m_loader = make_unique<objl::Loader>();
  m_loader->LoadFile(pathToModel);

  m_depthBuffer = new float[screenWidth * screenHeight];

  m_pipeline = make_unique<Pipeline>();

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
          vertex.position = { v.Position.X, v.Position.Y, v.Position.Z };
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
  delete[] m_depthBuffer;
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
  m_pipeline->SetRSDescriptor(m_screenWidth, m_screenHeight, Pipeline::Culling::CW);
  m_pipeline->SetOMBuffers(m_depthBuffer, m_renderTarget, m_screenWidth, m_screenHeight);
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
