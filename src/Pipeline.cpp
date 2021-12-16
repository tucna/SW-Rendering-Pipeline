#include <algorithm>

#include "Pipeline.h"
#include "../engine/tPixelGameEngine.h"

using namespace std;
using namespace math;

Pipeline::Pipeline(tDX::PixelGameEngine * engine) :
  m_engine(engine)
{
}

Pipeline::~Pipeline()
{
}

void Pipeline::ClearDepthBuffer()
{
  // TODO
  for (size_t i = 0; i < 800 * 600; i++)
    m_depthBuffer[i] = std::numeric_limits<float>::min();
}

void Pipeline::Draw()
{
  // TODO clear
  m_VSOutputs.clear();
  m_VSOutputTriangles.clear();
  // --------

  IA();

  // VS
  for (size_t vertexID = 0; vertexID < m_vertices->size(); vertexID++)
  {
    VSOutput output = VS(m_vertices->at(vertexID));

    PostVS(output);

    m_VSOutputs.push_back(output);
  }

  PA();

  for (auto& triangle : m_VSOutputTriangles)
    RS(triangle);
}

void Pipeline::IA()
{
  // Nothing to do in here

  /*
  for (size_t index = 0; index < m_indices->size(); index += 3)
  {
    Triangle triangle;

    triangle.v1.position = m_vertices->at(m_indices->at(index)).position;
    triangle.v1.normal = m_vertices->at(m_indices->at(index)).normal;
    triangle.v2.position = m_vertices->at(m_indices->at(index + 1)).position;
    triangle.v2.normal = m_vertices->at(m_indices->at(index + 1)).normal;
    triangle.v3.position = m_vertices->at(m_indices->at(index + 2)).position;
    triangle.v3.normal = m_vertices->at(m_indices->at(index + 2)).normal;

    m_triangles.push_back(triangle);
  }
  */
}

Pipeline::VSOutput Pipeline::VS(const Vertex& vertex)
{
  VSOutput output;

  output.position = m_mvpMatrix * float4(vertex.position, 1.0f);
  output.normal = vertex.normal;

  float4 v = m_viewMatrix * m_modelMatrix * float4(vertex.position, 1.0f);
  float4 n = m_viewMatrix * m_modelMatrix * float4(vertex.normal, 0.0f);

  float dotP = dot(normalize({ v.x, v.y, v.z }), normalize({ n.x, n.y, n.z }));
  output.viewDot = dotP > 0.0f ? 0.0f : abs(dotP);

  return output;


  /*
  for (auto& triangle : m_triangles)
  {
    float4 transV1, transV2, transV3;

    transV1 = m_mvpMatrix * float4(triangle.v1.position, 1.0f);
    transV2 = m_mvpMatrix * float4(triangle.v2.position, 1.0f);
    transV3 = m_mvpMatrix * float4(triangle.v3.position, 1.0f);

    float3 v1s = { screenV1.x, screenV1.y, 1.0f / v1.w };
    float3 v2s = { screenV2.x, screenV2.y, 1.0f / v2.w };
    float3 v3s = { screenV3.x, screenV3.y, 1.0f / v3.w };

    m_transposedTriangles.push_back({transV1, transV2, transV3});
  }
  */
}

void Pipeline::PostVS(VSOutput& vsoutput)
{
  // Transform to NDC - TODO check
  float invW = 1.0f / vsoutput.position.w;

  vsoutput.position.x *= invW;
  vsoutput.position.y *= invW;
  vsoutput.position.z = invW;
}

void Pipeline::PA()
{
  for (size_t index = 0; index < m_VSOutputs.size(); index += 3)
  {
    VSOutputTriangle triangle;

    // TODO lambda?
    triangle.v1.position = m_VSOutputs[index + 0].position;
    triangle.v1.normal   = m_VSOutputs[index + 0].normal;
    triangle.v1.viewDot  = m_VSOutputs[index + 0].viewDot;
    triangle.v2.position = m_VSOutputs[index + 1].position;
    triangle.v2.normal   = m_VSOutputs[index + 1].normal;
    triangle.v2.viewDot  = m_VSOutputs[index + 1].viewDot;
    triangle.v3.position = m_VSOutputs[index + 2].position;
    triangle.v3.normal   = m_VSOutputs[index + 2].normal;
    triangle.v3.viewDot  = m_VSOutputs[index + 2].viewDot;

    m_VSOutputTriangles.push_back(triangle);
  }
}

void Pipeline::RS(VSOutputTriangle& triangle)
{
  // TODO lambda?

  // To screen space
  triangle.v1.position.x = (triangle.v1.position.x + 1.0f) * m_viewportWidth * 0.5f;
  triangle.v1.position.y = (1.0f - triangle.v1.position.y) * m_viewportHeight * 0.5f;

  triangle.v2.position.x = (triangle.v2.position.x + 1.0f) * m_viewportWidth * 0.5f;
  triangle.v2.position.y = (1.0f - triangle.v2.position.y) * m_viewportHeight * 0.5f;

  triangle.v3.position.x = (triangle.v3.position.x + 1.0f) * m_viewportWidth * 0.5f;
  triangle.v3.position.y = (1.0f - triangle.v3.position.y) * m_viewportHeight * 0.5f;

  float3 v1 = { triangle.v1.position.x, triangle.v1.position.y, triangle.v1.position.z };
  float3 v2 = { triangle.v2.position.x, triangle.v2.position.y, triangle.v2.position.z };
  float3 v3 = { triangle.v3.position.x, triangle.v3.position.y, triangle.v3.position.z };

  // get the bounding box of the triangle
  int maxX = lround(std::max(v1.x, std::max(v2.x, v3.x)));
  int minX = lround(std::min(v1.x, std::min(v2.x, v3.x)));
  int maxY = lround(std::max(v1.y, std::max(v2.y, v3.y)));
  int minY = lround(std::min(v1.y, std::min(v2.y, v3.y)));

  maxX = std::clamp(maxX, 0, m_viewportWidth - 1);
  minX = std::clamp(minX, 0, m_viewportWidth - 1);
  maxY = std::clamp(maxY, 0, m_viewportHeight - 1);
  minY = std::clamp(minY, 0, m_viewportHeight - 1);

  // Barycentric interpolation
  float det = 1.0f / ((v2.y - v3.y) * (v1.x - v3.x) + (v3.x - v2.x) * (v1.y - v3.y));

  for (uint16_t x = minX; x <= maxX; x++)
  {
    for (uint16_t y = minY; y <= maxY; y++)
    {
      float w1 = ((v2.y - v3.y) * (x - v3.x) + (v3.x - v2.x) * (y - v3.y)) * det;
      float w2 = ((v3.y - v1.y) * (x - v3.x) + (v1.x - v3.x) * (y - v3.y)) * det;
      float w3 = 1.0f - w1 - w2;

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

        float4 color = PS(vertex);

        m_engine->Draw(x,y, {uint8_t(color.x * 255), uint8_t(color.y * 255), uint8_t(color.z * 255)});
      }
    }
  }
}

float4 Pipeline::PS(VSOutput& psinput)
{
  float4 color = { psinput.viewDot * m_Kd.x, psinput.viewDot * m_Kd.y, psinput.viewDot * m_Kd.z, 1 };
  return color;
}
