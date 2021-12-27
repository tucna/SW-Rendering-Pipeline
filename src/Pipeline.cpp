#include <algorithm>
#include <execution>

#include "Pipeline.h"

using namespace std;
using namespace math;

void Pipeline::Draw()
{
  // Clean all internal structures
  Cleanup();

  // IA
  InputAssembler();

  // VS
  for (auto& vertex : *m_vertices)
    m_VSOutputs.push_back(VertexShader(vertex));

  // PA
  PrimitiveAssembly();

  // RS, PS, OM
  std::for_each(std::execution::par_unseq, std::begin(m_VSOutputTriangles), std::end(m_VSOutputTriangles), [&](auto& triangle) { Rasterizer(triangle); });
}

void Pipeline::InputAssembler()
{
  // Nothing to do in here yet
}

Pipeline::VSOutput Pipeline::VertexShader(const Vertex& vertex)
{
  VSOutput output;

  output.position = m_mvpMatrix * float4(vertex.position, 1.0f);

  float4 wp = m_modelMatrix * float4(vertex.position, 1.0f);
  output.worldPosition = wp.xyz();

  float4 n = m_modelMatrix * float4(vertex.normal, 0.0f);
  output.normal = n.xyz();

  float4 b = m_modelMatrix * float4(vertex.bitangent, 0.0f);
  output.bitangent = b.xyz();

  float4 t = m_modelMatrix * float4(vertex.tangent, 0.0f);
  output.tangent = t.xyz();

  output.uv = vertex.uv;

  return output;
}

float4 Pipeline::PixelShader(VSOutput& psinput)
{
  //return float4(1,0,0,1);
  //return float4(normalize(psinput.normal) * 0.5f + 0.5f,1.0f);

  const float3 lightAmbient  = { 0.2f, 0.2f, 0.2f };
  const float3 lightDiffuse  = { 0.8f, 0.8f, 0.8f };
  const float3 lightSpecular = { 1.0f, 1.0f, 1.0f };

  float3 objectAmbient = { 0.1f, 0.1f, 0.1f };
  float3 objectDiffuse = { 1.0f, 1.0f, 1.0f };
  float3 objectSpecular = { 1.0f, 1.0f, 1.0f };

  if (m_textures->Ka_map)
    objectAmbient = sample(m_textures->Ka_map, { m_textures->texturesWidth, m_textures->texturesHeight }, psinput.uv);

  if (m_textures->Kd_map)
    objectDiffuse = sample(m_textures->Kd_map, { m_textures->texturesWidth, m_textures->texturesHeight }, psinput.uv);

  if (m_textures->Ks_map)
    objectSpecular = sample(m_textures->Ks_map, { m_textures->texturesWidth, m_textures->texturesHeight }, psinput.uv);

  float3 ambient = m_reflectance.Ka * lightAmbient * objectAmbient;

  float3 normal = normalize(psinput.normal);

  if (m_textures->Bump_map)
  {
    float3 N = normal;
    float3 B = normalize(psinput.bitangent);
    float3 T = normalize(psinput.tangent);

    float4x4 TBN =
    { {
      {{ T.x, B.x, N.x, 0 }},
      {{ T.y, B.y, N.y, 0 }},
      {{ T.z, B.z, N.z, 0 }},
      {{ 0  , 0  , 0  , 1 }},
    } };

    normal = sample(m_textures->Bump_map, { m_textures->texturesWidth, m_textures->texturesHeight }, psinput.uv);
    normal = normalize(normal * 2.0f - 1.0f);
    normal = normalize(float4(TBN * float4(normal, 0.0f)).xyz());
  }

  float3 lightDir = normalize(m_lightPosition - psinput.worldPosition);

  float diff = max(dot(normal, lightDir), 0.0f);
  float3 diffuse = m_reflectance.Kd * diff * lightDiffuse * objectDiffuse;

  float3 viewDir = normalize(m_cameraPosition - psinput.worldPosition);

  // Blinn-Phong
  float3 halfwayDir = normalize(lightDir + viewDir);
  float spec = pow(max(dot(normal, halfwayDir), 0.0f), 32);

  // Phong
  //float3 reflectDir = reflect(-lightDir, normal);
  //float spec = pow(max(dot(viewDir, reflectDir), 0.0f), 32);

  float3 specular = m_reflectance.Ks * spec * lightSpecular * objectSpecular;

  float3 result = saturate(ambient + diffuse + specular);
  return float4(result, 1.0f);
}

void Pipeline::PrimitiveAssembly()
{
  auto FillUpData = [&](VSOutput& vertex, size_t index)
  {
    vertex.position = m_VSOutputs[index].position;
    vertex.worldPosition = m_VSOutputs[index].worldPosition;
    vertex.normal   = m_VSOutputs[index].normal;
    vertex.tangent = m_VSOutputs[index].tangent;
    vertex.bitangent = m_VSOutputs[index].bitangent;
    vertex.uv = m_VSOutputs[index].uv;
  };

  for (size_t index = 0; index < m_VSOutputs.size(); index += 3)
  {
    VSOutputTriangle triangle;

    FillUpData(triangle.v1, index + 0);
    FillUpData(triangle.v2, index + 1);
    FillUpData(triangle.v3, index + 2);

    // Cull
    if (triangle.v1.position.x > triangle.v1.position.w &&
        triangle.v2.position.x > triangle.v2.position.w &&
        triangle.v3.position.x > triangle.v3.position.w)
      continue;

    if (triangle.v1.position.x < -triangle.v1.position.w &&
        triangle.v2.position.x < -triangle.v2.position.w &&
        triangle.v3.position.x < -triangle.v3.position.w)
      continue;

    if (triangle.v1.position.y > triangle.v1.position.w &&
        triangle.v2.position.y > triangle.v2.position.w &&
        triangle.v3.position.y > triangle.v3.position.w)
      continue;

    if (triangle.v1.position.y < -triangle.v1.position.w &&
        triangle.v2.position.y < -triangle.v2.position.w &&
        triangle.v3.position.y < -triangle.v3.position.w)
      continue;

    if (triangle.v1.position.z > triangle.v1.position.w &&
        triangle.v2.position.z > triangle.v2.position.w &&
        triangle.v3.position.z > triangle.v3.position.w)
      continue;

    if (triangle.v1.position.z < 0.0f &&
        triangle.v2.position.z < 0.0f &&
        triangle.v3.position.z < 0.0f)
      continue;

    // clipping routines
    const auto Clip1 = [&](VSOutput& v1, VSOutput& v2, VSOutput& v3)
    {
      // calculate alpha values for getting adjusted vertices
      const float alphaA = (-v1.position.z) / (v2.position.z - v1.position.z);
      const float alphaB = (-v1.position.z) / (v3.position.z - v1.position.z);
      // interpolate to get v0a and v0b
      const auto v0a = interpolate(v1, v2, alphaA);
      const auto v0b = interpolate(v1, v3, alphaB);

      m_VSOutputTriangles.push_back({ v0a, v2, v3 });
      m_VSOutputTriangles.push_back({ v0b, v0a, v3 });
    };

    const auto Clip2 = [&](VSOutput& v1, VSOutput& v2, VSOutput& v3)
    {
      // calculate alpha values for getting adjusted vertices
      const float alpha0 = (-v1.position.z) / (v3.position.z - v1.position.z);
      const float alpha1 = (-v2.position.z) / (v3.position.z - v2.position.z);
      // interpolate to get v0a and v0b
      v1 = interpolate(v1, v3, alpha0);
      v2 = interpolate(v2, v3, alpha1);

      m_VSOutputTriangles.push_back({ v1, v2, v3 });
    };

    // near clipping tests
    if (triangle.v1.position.z < 0.0f)
    {
      if (triangle.v2.position.z < 0.0f)
        Clip2(triangle.v1, triangle.v2, triangle.v3);
      else if (triangle.v3.position.z < 0.0f)
        Clip2(triangle.v1, triangle.v3, triangle.v2);
      else
        Clip1(triangle.v1, triangle.v2, triangle.v3);
    }
    else if (triangle.v2.position.z < 0.0f)
    {
      if (triangle.v3.position.z < 0.0f)
        Clip2(triangle.v2, triangle.v3, triangle.v1);
      else
        Clip1(triangle.v2, triangle.v1, triangle.v3);
    }
    else if (triangle.v3.position.z < 0.0f)
      Clip1(triangle.v3, triangle.v1, triangle.v2);
    else // no near clipping necessary
      m_VSOutputTriangles.push_back(triangle);

    //m_VSOutputTriangles.push_back(triangle);
  }

}

void Pipeline::Rasterizer(VSOutputTriangle& triangle)
{
  auto ToNDCAndPerspectiveCorrection = [](VSOutput& vertex)
  {
    float invW = 1.0f / vertex.position.w;

    // Everything from vsoutput must be processed TODO: template?
    vertex.position *= invW;
    vertex.normal *= invW;
    vertex.tangent *= invW;
    vertex.bitangent *= invW;
    vertex.uv *= invW;
    vertex.worldPosition *= invW;

    vertex.position.w = invW;
  };

  auto ToScreenSpace = [&](float4& position)
  {
    position.x = (position.x + 1.0f) * m_viewportWidth * 0.5f;
    position.y = (1.0f - position.y) * m_viewportHeight * 0.5f;
  };

  auto EdgeFunction = [](const float3& v1, const float3& v2, const float3& v3) -> float
  {
    return (v3.x - v1.x) * (v2.y - v1.y) - (v3.y - v1.y) * (v2.x - v1.x);
    //return (v1.x - v2.x) * (v3.y - v1.y) - (v1.y - v2.y) * (v3.x - v1.x);
  };

  // Transform
  ToNDCAndPerspectiveCorrection(triangle.v1);
  ToNDCAndPerspectiveCorrection(triangle.v2);
  ToNDCAndPerspectiveCorrection(triangle.v3);

  ToScreenSpace(triangle.v1.position);
  ToScreenSpace(triangle.v2.position);
  ToScreenSpace(triangle.v3.position);

  float3 v1 = { triangle.v1.position.x, triangle.v1.position.y, triangle.v1.position.z };
  float3 v2 = { triangle.v2.position.x, triangle.v2.position.y, triangle.v2.position.z };
  float3 v3 = { triangle.v3.position.x, triangle.v3.position.y, triangle.v3.position.z };

  // Culling
  float area = EdgeFunction(v1, v2, v3);

  if ((m_culling == Culling::CW && area < 0.0f) || (m_culling == Culling::CCW && area > 0.0f))
    return;

  // Get the bounding box of the triangle
  int maxX = lround(std::max(v1.x, std::max(v2.x, v3.x)));
  int minX = lround(std::min(v1.x, std::min(v2.x, v3.x)));
  int maxY = lround(std::max(v1.y, std::max(v2.y, v3.y)));
  int minY = lround(std::min(v1.y, std::min(v2.y, v3.y)));

  maxX = std::clamp(maxX, 0, m_viewportWidth - 1);
  minX = std::clamp(minX, 0, m_viewportWidth - 1);
  maxY = std::clamp(maxY, 0, m_viewportHeight - 1);
  minY = std::clamp(minY, 0, m_viewportHeight - 1);

  for (uint16_t x = minX; x <= maxX; x++)
  {
    for (uint16_t y = minY; y <= maxY; y++)
    {
      // Barycentric interpolation
      float3 p = {x + 0.5f, y + 0.5f, 0.0f};
      float w1 = EdgeFunction(v2, v3, p);
      float w2 = EdgeFunction(v3, v1, p);
      float w3 = EdgeFunction(v1, v2, p);

      if (w1 < 0 || w2 < 0 || w3 < 0)
        continue;

      w1 /= area;
      w2 /= area;
      w3 /= area;

      float z = w1 * v1.z + w2 * v2.z + w3 * v3.z;

      if (z < m_depthBuffer[y * m_buffersWidth + x])
      {
        m_depthBuffer[y * m_buffersWidth + x] = z;

        float w = w1 * triangle.v1.position.w + w2 * triangle.v2.position.w + w3 * triangle.v3.position.w;
        float invW = 1.0f / w;

        VSOutput vertex;
        vertex.position = { (float)x, (float)y, z, w };

        vertex.worldPosition = w1 * triangle.v1.worldPosition + w2 * triangle.v2.worldPosition + w3 * triangle.v3.worldPosition;
        vertex.normal = w1 * triangle.v1.normal + w2 * triangle.v2.normal + w3 * triangle.v3.normal;
        vertex.tangent = w1 * triangle.v1.tangent + w2 * triangle.v2.tangent + w3 * triangle.v3.tangent;
        vertex.bitangent = w1 * triangle.v1.bitangent + w2 * triangle.v2.bitangent + w3 * triangle.v3.bitangent;
        vertex.uv = w1 * triangle.v1.uv + w2 * triangle.v2.uv + w3 * triangle.v3.uv;

        vertex.worldPosition *= invW;
        vertex.normal *= invW;
        vertex.tangent *= invW;
        vertex.bitangent *= invW;
        vertex.uv *= invW;

        float4 color = PixelShader(vertex);
        OutputMerger(x, y, color);
      }
    }
  }
}

void Pipeline::OutputMerger(uint16_t x, uint16_t y, float4 color)
{
  m_renderTarget[y * m_buffersWidth + x] = {
    (uint8_t)(lround(color.r * 255)),
    (uint8_t)(lround(color.g * 255)),
    (uint8_t)(lround(color.b * 255)),
    (uint8_t)(lround(color.a * 255))
  };
}

void Pipeline::Cleanup()
{
  m_VSOutputs.clear();
  m_VSOutputTriangles.clear();
}
