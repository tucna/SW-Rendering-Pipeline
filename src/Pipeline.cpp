#include <algorithm>

#include "Pipeline.h"

using namespace std;
using namespace math;

void Pipeline::ClearDepthBuffer()
{
  for (size_t i = 0; i < m_buffersWidth * m_buffersHeight; i++)
    m_depthBuffer[i] = std::numeric_limits<float>::min();
}

void Pipeline::Draw()
{
  // Clean all internal structures
  Cleanup();

  InputAssembler();

  // VS
  for (size_t vertexID = 0; vertexID < m_vertices->size(); vertexID++)
  {
    VSOutput output = VertexShader(m_vertices->at(vertexID));

    PostVertexShader(output);

    m_VSOutputs.push_back(output);
  }

  PrimitiveAssembly();

  for (auto& triangle : m_VSOutputTriangles)
    Rasterizer(triangle);
}

void Pipeline::InputAssembler()
{
  // Nothing to do in here yet
}

Pipeline::VSOutput Pipeline::VertexShader(const Vertex& vertex)
{
  VSOutput output;

  output.position = m_mvpMatrix * float4(vertex.position, 1.0f);

  float4 wp = m_modelMatrix * float4(vertex.position, 1.0f);
  output.worldPosition = { wp.x, wp.y, wp.z };

  float4 n = m_modelMatrix * float4(vertex.normal, 0.0f);
  output.normal = {n.x, n.y, n.z};

  output.uv = vertex.uv;

  return output;
}

float4 Pipeline::PixelShader(VSOutput& psinput)
{
  return {psinput.uv.y, psinput.uv.x, 0, 1};

  float3 lightColor = {1.0f, 1.0f, 1.0f};
  float3 objectColor = {0.8f, 0.5f, 0.2f};

  float ambientStrength = 0.1f;
  float3 ambient = ambientStrength * lightColor;

  float3 normal = normalize(psinput.normal);
  float3 lightDir = normalize(m_lightPosition - psinput.worldPosition);

  float diff = max(dot(normal, lightDir), 0.0f);
  float3 diffuse = diff * lightColor;

  float specularStrength = 0.5f;
  float3 viewDir = normalize(m_cameraPosition - psinput.worldPosition);
  float3 reflectDir = reflect(-lightDir, normal);
  float spec = pow(max(dot(viewDir, reflectDir), 0.0f), 32);
  float3 specular = specularStrength * spec * lightColor;

  float3 result = saturate((ambient + diffuse + specular)) * objectColor;
  return float4(result, 1.0f);

  /*
  float3 lightDir = normalize(m_lightPosition - psinput.worldPosition);
  float3 viewDir = normalize(m_cameraPosition - psinput.worldPosition);
  float3 halfwayDir = normalize(lightDir + viewDir);

  float3 n = normalize(psinput.normal);

  // Diffuse
  float diffuse = saturate(dot(n, lightDir));

  // Fall-off
  diffuse *= ((length(lightDir) * length(lightDir)) / dot(m_lightPosition - psinput.worldPosition, m_lightPosition - psinput.worldPosition));

  // Specular
  //float3 h = normalize(normalize(m_cameraPosition - psinput.worldPosition) - lightDir);
  //float specular = pow(saturate(dot(h, n)), 2.0f);
  float specular = pow(max(dot(n, halfwayDir), 0.0f), 2.0f);

  float4 color;
  color.r = saturate(m_Ka.x + (m_Kd.x * diffuse * 0.6f) + (specular * 0.5f));
  color.g = saturate(m_Ka.y + (m_Kd.y * diffuse * 0.6f) + (specular * 0.5f));
  color.b = saturate(m_Ka.z + (m_Kd.z * diffuse * 0.6f) + (specular * 0.5f));
  color.a = 1.0f;

  return color;
  */
}

void Pipeline::PostVertexShader(VSOutput& vsoutput)
{
  // Transform to NDC
  float invW = 1.0f / vsoutput.position.w;

  vsoutput.position.x *= invW;
  vsoutput.position.y *= invW;
  vsoutput.position.z = invW;
}

void Pipeline::PrimitiveAssembly()
{
  auto FillUpData = [&](VSOutput& vertex, size_t index)
  {
    vertex.position = m_VSOutputs[index].position;
    vertex.worldPosition = m_VSOutputs[index].worldPosition;
    vertex.normal   = m_VSOutputs[index].normal;
    vertex.uv = m_VSOutputs[index].uv;
  };

  for (size_t index = 0; index < m_VSOutputs.size(); index += 3)
  {
    VSOutputTriangle triangle;

    FillUpData(triangle.v1, index + 0);
    FillUpData(triangle.v2, index + 1);
    FillUpData(triangle.v3, index + 2);

    m_VSOutputTriangles.push_back(triangle);
  }
}

void Pipeline::Rasterizer(VSOutputTriangle& triangle)
{
  auto ToScreenSpace = [&](float4& position)
  {
    position.x = (position.x + 1.0f) * m_viewportWidth * 0.5f;
    position.y = (1.0f - position.y) * m_viewportHeight * 0.5f;
  };

  auto EdgeFunction = [](const float3& v1, const float3& v2, const float3& v3) -> float
  {
    return (v3.x - v1.x) * (v2.y - v1.y) - (v3.y - v1.y) * (v2.x - v1.x);
  };

  ToScreenSpace(triangle.v1.position);
  ToScreenSpace(triangle.v2.position);
  ToScreenSpace(triangle.v3.position);

  float3 v1 = { triangle.v1.position.x, triangle.v1.position.y, triangle.v1.position.z };
  float3 v2 = { triangle.v2.position.x, triangle.v2.position.y, triangle.v2.position.z };
  float3 v3 = { triangle.v3.position.x, triangle.v3.position.y, triangle.v3.position.z };

  // Culling
  float area = EdgeFunction(v1, v2, v3);

  if ((m_culling == Culling::CW && area < 0.0f) || (m_culling == Culling::CCW && area > 0.0f))
    return;

  // Get the bounding box of the triangle
  int maxX = lround(std::max(v1.x, std::max(v2.x, v3.x)));
  int minX = lround(std::min(v1.x, std::min(v2.x, v3.x)));
  int maxY = lround(std::max(v1.y, std::max(v2.y, v3.y)));
  int minY = lround(std::min(v1.y, std::min(v2.y, v3.y)));

  maxX = std::clamp(maxX, 0, m_viewportWidth - 1);
  minX = std::clamp(minX, 0, m_viewportWidth - 1);
  maxY = std::clamp(maxY, 0, m_viewportHeight - 1);
  minY = std::clamp(minY, 0, m_viewportHeight - 1);

  for (uint16_t x = minX; x <= maxX; x++)
  {
    for (uint16_t y = minY; y <= maxY; y++)
    {
      // Barycentric interpolation
      float3 p = {x + 0.5f, y + 0.5f, 0.0f};
      float w1 = EdgeFunction(v2, v3, p) / area;
      float w2 = EdgeFunction(v3, v1, p) / area;
      float w3 = EdgeFunction(v1, v2, p) / area;

      if (w1 < 0 || w2 < 0 || w3 < 0)
        continue;

      float z = w1 * v1.z + w2 * v2.z + w3 * v3.z;

      if (z > m_depthBuffer[y * m_buffersWidth + x])
      {
        m_depthBuffer[y * m_buffersWidth + x] = z;

        VSOutput vertex;
        vertex.position = { (float)x, (float)y, z, z }; // TODO second "z" is not correct - 1/z instead?
        vertex.worldPosition = w1 * triangle.v1.worldPosition + w2 * triangle.v2.worldPosition + w3 * triangle.v3.worldPosition;
        vertex.normal = w1 * triangle.v1.normal + w2 * triangle.v2.normal + w3 * triangle.v3.normal;
        vertex.uv = w1 * triangle.v1.uv + w2 * triangle.v2.uv + w3 * triangle.v3.uv;

        float4 color = PixelShader(vertex);
        OutputMerger(x, y, color);
      }
    }
  }
}

void Pipeline::OutputMerger(uint16_t x, uint16_t y, float4 color)
{
  m_renderTarget[y * m_buffersWidth + x] = {
    (uint8_t)(lround(color.r * 255)),
    (uint8_t)(lround(color.g * 255)),
    (uint8_t)(lround(color.b * 255)),
    (uint8_t)(lround(color.a * 255))
  };
}

void Pipeline::Cleanup()
{
  m_VSOutputs.clear();
  m_VSOutputTriangles.clear();
}
