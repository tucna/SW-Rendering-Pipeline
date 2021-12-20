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

  void SetIAInput(const std::vector<math::Vertex>& vertices, const std::vector<uint32_t>& indices) { m_vertices = &vertices; m_indices = &indices; }
  void SetVSBuffers(math::float4x4 mvpMatrix, math::float4x4 viewMatrix, math::float4x4 modelMatrix) { m_mvpMatrix = mvpMatrix; m_viewMatrix = viewMatrix; m_modelMatrix = modelMatrix; }
  void SetRSDescriptor(uint16_t viewportWidth, uint16_t viewportHeight, Culling culling) { m_viewportWidth = viewportWidth; m_viewportHeight = viewportHeight; m_culling = culling; }
  void SetPSBuffers(math::float3 Kd, math::float3 Ka, math::float3 lightPosition, math::float3 cameraPosition, math::byte4* texture0, uint16_t texture0size) { m_Kd = Kd; m_Ka = Ka; m_lightPosition = lightPosition; m_cameraPosition = cameraPosition; m_texture0 = texture0; m_texture0size = texture0size; }
  void SetOMBuffers(float* depthBuffer, math::byte4* renderTarget, uint16_t buffersWidth, uint16_t buffersHeight) { m_depthBuffer = depthBuffer; m_renderTarget = renderTarget; m_buffersWidth = buffersWidth; m_buffersHeight = buffersHeight; }

  void ClearDepthBuffer();
  void Draw();

private:
  struct VSOutput
  {
    math::float4 position;
    math::float3 worldPosition;
    math::float2 uv;
    math::float3 normal;
  };

  struct VSOutputTriangle
  {
    VSOutput v1, v2, v3;
  };

  void InputAssembler();
  VSOutput VertexShader(const math::Vertex& vertex);
  void PostVertexShader(VSOutput& vsoutput);
  void PrimitiveAssembly();
  void Rasterizer(VSOutputTriangle& triangle);
  math::float4 PixelShader(VSOutput& psinput);
  void OutputMerger(uint16_t x, uint16_t y, math::float4 color);

  void Cleanup();

  // IA
  const std::vector<math::Vertex>* m_vertices;
  const std::vector<uint32_t>* m_indices;

  // VS
  math::float4x4 m_mvpMatrix;
  math::float4x4 m_viewMatrix;
  math::float4x4 m_modelMatrix;
  std::vector<VSOutput> m_VSOutputs;

  // PA
  std::vector<VSOutputTriangle> m_VSOutputTriangles;

  // RS
  uint16_t m_viewportWidth;
  uint16_t m_viewportHeight;
  Culling m_culling = Culling::None;

  // PS
  math::float3 m_Kd;
  math::float3 m_Ka;
  math::float3 m_lightPosition;
  math::float3 m_cameraPosition;
  math::byte4* m_texture0 = nullptr;
  uint16_t m_texture0size;

  // OM
  float* m_depthBuffer = nullptr;
  math::byte4* m_renderTarget = nullptr;
  uint16_t m_buffersWidth;
  uint16_t m_buffersHeight;

  //std::vector<std::thread> m_threads;
};

