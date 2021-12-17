#include <algorithm>

#include "Pipeline.h"
#include "../engine/tPixelGameEngine.h"

using namespace std;
using namespace math;

void Pipeline::ClearDepthBuffer()
{
  // TODO
  for (size_t i = 0; i < 800 * 600; i++)
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
  output.normal = vertex.normal;

  float4 v = m_viewMatrix * m_modelMatrix * float4(vertex.position, 1.0f);
  float4 n = m_viewMatrix * m_modelMatrix * float4(vertex.normal, 0.0f);

  float dotP = dot(normalize({ v.x, v.y, v.z }), normalize({ n.x, n.y, n.z }));
  output.viewDot = abs(dotP);

  return output;
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
    vertex.normal   = m_VSOutputs[index].normal;
    vertex.viewDot  = m_VSOutputs[index].viewDot;
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

      if (z > m_depthBuffer[y * 800 + x]) // TUCNA
      {
        m_depthBuffer[y * 800 + x] = z;

        VSOutput vertex;
        vertex.position = { (float)x, (float)y, z, z }; // TODO second "z" is not correct - 1/z instead?
        vertex.normal = triangle.v1.normal; // TODO interpolate normals
        vertex.viewDot = triangle.v1.viewDot;

        float4 color = PixelShader(vertex);

        m_renderTarget[y * (800 * 4) + (x * 4) + 0] = (uint8_t)(lround(color.x * 255));
        m_renderTarget[y * (800 * 4) + (x * 4) + 1] = (uint8_t)(lround(color.y * 255));
        m_renderTarget[y * (800 * 4) + (x * 4) + 2] = (uint8_t)(lround(color.z * 255));
        m_renderTarget[y * (800 * 4) + (x * 4) + 3] = (uint8_t)(lround(color.w * 255));
      }
    }
  }
}

float4 Pipeline::PixelShader(VSOutput& psinput)
{
  float4 color = { psinput.viewDot * m_Kd.x + m_Ka.x, psinput.viewDot * m_Kd.y + m_Ka.y, psinput.viewDot * m_Kd.z + m_Ka.z, 1.0f };

  color.r = min(color.r, 1.0f);
  color.g = min(color.g, 1.0f);
  color.b = min(color.b, 1.0f);

  return color;
}

void Pipeline::Cleanup()
{
  m_VSOutputs.clear();
  m_VSOutputTriangles.clear();
}
