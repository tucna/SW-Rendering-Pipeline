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
    m_frame = make_unique<Scene>("res/cornell_box.obj");
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
    if (GetKey(tDX::W).bHeld) { m_frame->MoveCamera({ 0, 0, -coeficient}); }
    if (GetKey(tDX::S).bHeld) { m_frame->MoveCamera({ 0, 0, coeficient }); }
    if (GetKey(tDX::E).bHeld) { m_frame->RotateModel({ 0, coeficient * 15, 0 }); }
    if (GetKey(tDX::Q).bHeld) { m_frame->RotateModel({ 0, -coeficient * 15, 0 }); }

    const vector<triangle>& triangles = m_frame->GenerateTrianglesToDraw();

    for (const auto& t : triangles)
    {
      // TODO
      tDX::vf3d v1 = { t.v1.x, t.v1.y, t.v1.z };
      tDX::vf3d v2 = { t.v2.x, t.v2.y, t.v2.z };
      tDX::vf3d v3 = { t.v3.x, t.v3.y, t.v3.z };
      tDX::Pixel c = { t.r, t.g, t.b, 255 };

      FillTriangleTUCNA(v1, v2, v3, c);
    }

    return true;
  }

private:
  unique_ptr<Scene> m_frame;
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
  Light light;
  if (light.Construct(800, 600, 1, 1))
    light.Start();

  return 0;
}