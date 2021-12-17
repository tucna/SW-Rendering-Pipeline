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

class Scene
{
public:
  Scene(const std::string& pathToModel, byte4* renderTarger, uint16_t screenWidth, uint16_t screenHeight);
  ~Scene();

  void MoveCamera(float3 translation);
  void MoveLight(float3 translation);
  void RotateModel(float3 rotation);

  void ComposeMatrices();

  void Draw();

private:
  void PlaceModelToCenter();

  std::unique_ptr<objl::Loader> m_loader;
  std::unique_ptr<Pipeline> m_pipeline;

  // Matrices
  float3 m_translation = { 0, 0, 0 };
  float3 m_rotation = { 0, 0, 0 };

  float4x4 m_modelMatrix;
  float4x4 m_viewMatrix;
  float4x4 m_projectionMatrix;

  float4x4 m_mvpMatrix;

  // Light
  float3 m_lightTranslation = { 0, 0, 0 };
  float4x4 m_mvpLightMatrix;

  // Look at
  float3 m_eye = { 0, 0, 0 };
  float3 m_target = { 0, 0, -1 };
  float3 m_up = { 0, 1, 0 };

  byte4* m_renderTarget;

  std::vector<std::vector<Vertex>> m_sortedVerticesByMaterial;
  std::vector< std::vector<uint32_t>> m_sortedIndicesByMaterial;

  float* m_depthBuffer;
  float m_aspectRatio;

  uint16_t m_screenWidth;
  uint16_t m_screenHeight;
};
