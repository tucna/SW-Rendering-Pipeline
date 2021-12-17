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
  enum class Culling
  {
    CW,
    CCW,
    None
  };

  void SetIAInput(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices) { m_vertices = &vertices; m_indices = &indices; }
  void SetVSBuffers(float4x4 mvpMatrix, float4x4 viewMatrix, float4x4 modelMatrix) { m_mvpMatrix = mvpMatrix; m_viewMatrix = viewMatrix; m_modelMatrix = modelMatrix; }
  void SetRSDescriptor(uint16_t viewportWidth, uint16_t viewportHeight, Culling culling) { m_viewportWidth = viewportWidth; m_viewportHeight = viewportHeight; m_culling = culling; }
  void SetPSBuffers(float3 Kd, float3 Ka) { m_Kd = Kd; m_Ka = Ka; }
  void SetOMBuffers(float* depthBuffer, byte4* renderTarget, uint16_t buffersWidth, uint16_t buffersHeight) { m_depthBuffer = depthBuffer; m_renderTarget = renderTarget; m_buffersWidth = buffersWidth; m_buffersHeight = buffersHeight; } // TUCNA should not be necessary to save

  void ClearDepthBuffer();
  void Draw();

private:
  struct VSOutput
  {
    float4 position;
    float3 normal;
    float viewDot;
  };

  struct VSOutputTriangle
  {
    VSOutput v1, v2, v3;
  };

  void InputAssembler();
  VSOutput VertexShader(const Vertex& vertex);
  void PostVertexShader(VSOutput& vsoutput);
  void PrimitiveAssembly(); //primitive assembly
  void Rasterizer(VSOutputTriangle& triangle); //rasterizer
  float4 PixelShader(VSOutput& psinput);
  //void OM();

  void Cleanup();

  // IA
  const std::vector<Vertex>* m_vertices;
  const std::vector<uint32_t>* m_indices;

  // VS
  float4x4 m_mvpMatrix;
  float4x4 m_viewMatrix;
  float4x4 m_modelMatrix;
  std::vector<VSOutput> m_VSOutputs;

  // PA
  std::vector<VSOutputTriangle> m_VSOutputTriangles;

  // RS
  uint16_t m_viewportWidth;
  uint16_t m_viewportHeight;
  Culling m_culling = Culling::None;

  // PS
  float3 m_Kd;
  float3 m_Ka;

  // OM
  float* m_depthBuffer = nullptr;
  byte4* m_renderTarget = nullptr;
  uint16_t m_buffersWidth;
  uint16_t m_buffersHeight;
};

