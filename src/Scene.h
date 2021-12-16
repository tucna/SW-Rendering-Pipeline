#pragma once

#include <string>
#include <memory>
#include <vector>

#include "Math.h"
#include "Pipeline.h"

namespace objl
{
  class Loader;
}

namespace tDX
{
  class PixelGameEngine;
}

class Scene
{
public:
  Scene(const std::string& pathToModel, tDX::PixelGameEngine* engine);
  ~Scene();

  void MoveCamera(float3 translation);
  void RotateModel(float3 rotation);

  void ComposeMatrices();

  void Draw();

  std::vector<Triangle> GenerateTrianglesToDraw();

private:
  constexpr static float m_aspectRatio = 800.0f / 600.0f; // TODO

  void PlaceModelToCenter();

  std::unique_ptr<objl::Loader> m_loader;

  float3 m_translation = { 0, 0, 0 };
  float3 m_rotation = { 0, 0, 0 };

  float4x4 m_modelMatrix;
  float4x4 m_viewMatrix;
  float4x4 m_projectionMatrix;

  float4x4 m_mvpMatrix;

  // Look at
  float3 m_eye = { 0, 0, 0 };
  float3 m_target = { 0, 0, -1 };
  float3 m_up = { 0, 1, 0 };

  std::unique_ptr<Pipeline> m_pipeline;
  std::vector<std::vector<Vertex>> m_sortedVerticesByMaterial;
  std::vector< std::vector<uint32_t>> m_sortedIndicesByMaterial;

  float* m_depthBuffer = nullptr;

  // TODO bad smell!
  tDX::PixelGameEngine* m_engine;
};
