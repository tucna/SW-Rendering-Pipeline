#pragma once

#include "Math.h"

#include <vector>

class Pipeline
{
public:
  void SetIAInput(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices) { m_vertices = &vertices; m_indices = &indices; }
  void SetVSBuffers(float4x4 mvpMatrix) { m_mvpMatrix = mvpMatrix; }
  void SetRSDescriptor(uint16_t viewportWidth, uint16_t viewportHeight) { m_viewportWidth = viewportWidth; m_viewportHeight = viewportHeight; }

  void Draw();

private:
  struct VSOutput
  {
    float4 position;
    float3 normal;
  };

  struct VSOutputTriangle
  {
    VSOutput v1, v2, v3;
  };

  void IA();
  VSOutput VS(const Vertex& vertex);
  void PostVS(VSOutput& vsoutput);
  void PA(); //primitive assembly
  void RS(VSOutputTriangle& triangle); //rasterizer
  //void PS();

  // IA
  const std::vector<Vertex>* m_vertices;
  const std::vector<uint32_t>* m_indices;

  // VS
  float4x4 m_mvpMatrix;
  std::vector<VSOutput> m_VSOutputs;

  // PA
  std::vector<VSOutputTriangle> m_VSOutputTriangles;

  // RS
  uint16_t m_viewportWidth;
  uint16_t m_viewportHeight;
};

