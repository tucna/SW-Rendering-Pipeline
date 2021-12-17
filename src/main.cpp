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
  Light() :
    renderTarget(800,600)
  {
    sAppName = "Light";
    m_scene = make_unique<Scene>("res/cornell_box.obj", (uint8_t*)renderTarget.GetData(), this);
    //m_scene = make_unique<Scene>("res/nefertiti.obj", this);
  }

  bool OnUserCreate() override
  {
    SetDrawTarget(&renderTarget);

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

    m_scene->ComposeMatrices();
    m_scene->Draw();

    return true;
  }

private:
  unique_ptr<Scene> m_scene;

  tDX::Sprite renderTarget; // TUCNA should not be there
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
  Light light;
  if (light.Construct(800, 600, 1, 1))
    light.Start();

  return 0;
}