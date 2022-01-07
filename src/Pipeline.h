#pragma once

#include "Math.h"

#include <array>
#include <atomic>
#include <mutex>
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

  void SetIAInput(const std::vector<math::Vertex>& vertices, const std::vector<uint32_t>& indices) { m_vertices = &vertices; m_indices = &indices; } // m_indices currently not used
  void SetVSBuffers(math::float4x4& mvpMatrix, math::float4x4& modelMatrix, math::float4x4& viewMatrix, math::float4x4& projectionMatrix) { m_mvpMatrix = mvpMatrix; m_viewMatrix = viewMatrix; m_modelMatrix = modelMatrix; m_projectionMatrix = projectionMatrix; }
  void SetRSDescriptor(uint16_t viewportWidth, uint16_t viewportHeight, Culling culling) { m_viewportWidth = viewportWidth; m_viewportHeight = viewportHeight; m_culling = culling; } // culling currently cannot be changed
  void SetPSBuffers(const math::MaterialReflectance& reflectance, math::MaterialTextures* textures, math::float3 lightPosition, math::float3 cameraPosition) { m_reflectance = reflectance; m_textures = textures; m_lightPosition = lightPosition; m_cameraPosition = cameraPosition; }
  //void SetOMBuffers(std::vector<std::atomic<float>>* depthBuffer, math::byte4* renderTarget, uint16_t buffersWidth, uint16_t buffersHeight) { m_depthBuffer = depthBuffer; m_renderTarget = renderTarget; m_buffersWidth = buffersWidth; m_buffersHeight = buffersHeight; }
  void SetOMBuffers(float* depthBuffer, math::byte4* renderTarget, uint16_t buffersWidth, uint16_t buffersHeight) { m_depthBuffer = depthBuffer; m_renderTarget = renderTarget; m_buffersWidth = buffersWidth; m_buffersHeight = buffersHeight; }

  void Draw();

private:
  void InputAssembler();
  VSOutput VertexShader(const math::Vertex& vertex);
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
  math::float4x4 m_projectionMatrix;
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
  //std::vector<std::atomic<float>>* m_depthBuffer;

  math::byte4* m_renderTarget = nullptr;
  uint16_t m_buffersWidth;
  uint16_t m_buffersHeight;

  uint16_t m_overdraw = 0;
  std::mutex m_mtx;
};

// Operators needed for linear interpolation

inline Pipeline::VSOutput operator-(const Pipeline::VSOutput& v1, const Pipeline::VSOutput& v2)
{
  Pipeline::VSOutput result = {};

  result.position = v1.position - v2.position;
  result.worldPosition = v1.worldPosition - v2.worldPosition;
  result.normal = v1.normal - v2.normal;
  result.bitangent = v1.bitangent - v2.bitangent;
  result.tangent = v1.tangent - v2.tangent;
  result.uv = v1.uv - v2.uv;

  return result;
}

inline Pipeline::VSOutput operator+(const Pipeline::VSOutput& v1, const Pipeline::VSOutput& v2)
{
  Pipeline::VSOutput result = {};

  result.position = v1.position + v2.position;
  result.worldPosition = v1.worldPosition + v2.worldPosition;
  result.normal = v1.normal + v2.normal;
  result.bitangent = v1.bitangent + v2.bitangent;
  result.tangent = v1.tangent + v2.tangent;
  result.uv = v1.uv + v2.uv;

  return result;
}

inline Pipeline::VSOutput operator*(const Pipeline::VSOutput& v1, const Pipeline::VSOutput& v2)
{
  Pipeline::VSOutput result = {};

  result.position = v1.position * v2.position;
  result.worldPosition = v1.worldPosition * v2.worldPosition;
  result.normal = v1.normal * v2.normal;
  result.bitangent = v1.bitangent * v2.bitangent;
  result.tangent = v1.tangent * v2.tangent;
  result.uv = v1.uv * v2.uv;

  return result;
}

inline Pipeline::VSOutput operator*(const Pipeline::VSOutput& v1, float s1)
{
  Pipeline::VSOutput result = {};

  result.position = v1.position * s1;
  result.worldPosition = v1.worldPosition * s1;
  result.normal = v1.normal * s1;
  result.bitangent = v1.bitangent * s1;
  result.tangent = v1.tangent * s1;
  result.uv = v1.uv * s1;

  return result;
}
