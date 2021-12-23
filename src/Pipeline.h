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
  void SetPSBuffers(const math::MaterialReflectance& reflectance, math::MaterialTextures* textures, math::float3 lightPosition, math::float3 cameraPosition) { m_reflectance = reflectance; m_textures = textures; m_lightPosition = lightPosition; m_cameraPosition = cameraPosition; }
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
    math::float3 tangent;
    math::float3 bitangent;
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
  math::float3 m_lightPosition;
  math::float3 m_cameraPosition;
  math::MaterialTextures* m_textures;
  math::MaterialReflectance m_reflectance;

  // OM
  float* m_depthBuffer = nullptr;
  math::byte4* m_renderTarget = nullptr;
  uint16_t m_buffersWidth;
  uint16_t m_buffersHeight;

  //std::vector<std::thread> m_threads;
};

