#include "Pipeline.h"

using namespace std;

void Pipeline::Draw()
{
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

    triangle.v1.position = m_VSOutputs[index + 0].position;
    triangle.v1.normal   = m_VSOutputs[index + 0].normal;
    triangle.v2.position = m_VSOutputs[index + 1].position;
    triangle.v2.normal   = m_VSOutputs[index + 1].normal;
    triangle.v3.position = m_VSOutputs[index + 2].position;
    triangle.v3.normal   = m_VSOutputs[index + 2].normal;

    m_VSOutputTriangles.push_back(triangle);
  }
}

void Pipeline::RS(VSOutputTriangle& triangle)
{
}
