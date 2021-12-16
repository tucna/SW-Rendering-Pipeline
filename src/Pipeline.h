#pragma once

#include "Math.h"

#include <vector>

namespace tDX
{
  class PixelGameEngine;
}

class Pipeline
{
public:
  Pipeline(tDX::PixelGameEngine* engine); // TODO TUCNA terrible
  ~Pipeline();

  void SetIAInput(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices) { m_vertices = &vertices; m_indices = &indices; }
  void SetVSBuffers(float4x4 mvpMatrix) { m_mvpMatrix = mvpMatrix; }
  void SetRSDescriptor(uint16_t viewportWidth, uint16_t viewportHeight) { m_viewportWidth = viewportWidth; m_viewportHeight = viewportHeight; }
  void SetPSBuffers(float3 Kd)
  { 
    m_Kd = Kd;
  }
  void SetOMBuffers(float* depthBuffer) { m_depthBuffer = depthBuffer; }

  void ClearDepthBuffer();
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
  float4 PS(VSOutput& psinput);
  //void OM();

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

  // PS
  float3 m_Kd;

  // OM
  float* m_depthBuffer = nullptr;

  // TODO delete
  tDX::PixelGameEngine* m_engine;
};

