#define T_PGE_APPLICATION
#include "../engine/tPixelGameEngine.h"

#include "Scene.h"
#include "Math.h"

#include <memory>
#include <vector>

using namespace std;
using namespace math;

class Light : public tDX::PixelGameEngine
{
public:
  Light()
  {
    sAppName = "Light";
    m_scene = make_unique<Scene>("res/cornell_box.obj");
  }

  bool OnUserCreate() override
  {
    return true;
  }

  bool OnUserUpdate(float fElapsedTime) override
  {
    Clear(tDX::BLACK);

    // Keyboard control
    float coeficient = 10.0f * fElapsedTime;

    if (GetKey(tDX::SHIFT).bHeld) { coeficient *= 8; }
    if (GetKey(tDX::W).bHeld) { m_scene->MoveCamera({ 0, 0, -coeficient}); }
    if (GetKey(tDX::S).bHeld) { m_scene->MoveCamera({ 0, 0, coeficient }); }
    if (GetKey(tDX::E).bHeld) { m_scene->RotateModel({ 0, coeficient * 15, 0 }); }
    if (GetKey(tDX::Q).bHeld) { m_scene->RotateModel({ 0, -coeficient * 15, 0 }); }

    m_scene->Draw();
    /*
    const vector<Triangle>& triangles = m_scene->GenerateTrianglesToDraw();

    for (const auto& t : triangles)
    {
      // TODO
      tDX::vf3d v1 = { t.v1.position.x, t.v1.position.y, t.v1.position.z };
      tDX::vf3d v2 = { t.v2.position.x, t.v2.position.y, t.v2.position.z };
      tDX::vf3d v3 = { t.v3.position.x, t.v3.position.y, t.v3.position.z };
      tDX::Pixel c = { (uint8_t)lround(t.v1.color.x * 255), (uint8_t)lround(t.v1.color.y * 255), (uint8_t)lround(t.v1.color.z * 255), 255 };

      FillTriangleTUCNA(v1, v2, v3, c);
    }
    */
    return true;
  }

private:
  unique_ptr<Scene> m_scene;
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
  Light light;
  if (light.Construct(800, 600, 1, 1))
    light.Start();

  return 0;
}