#include "effect.hpp"

#include <glm/glm.hpp>

#include <random>

namespace util {
  inline int RandomInt() {
    static std::mt19937 engine{std::random_device{}()};
    static std::uniform_int_distribution<int> dist;
    return dist(engine);
  }
} // namespace util

void Effect::Init(int width, int height) {
  scrollShader.CreateCompute("res/Shader/noice/scroll_move.comp.glsl");
  fillShader.CreateCompute("res/Shader/noice/scroll_fill.comp.glsl");

  //modelSSB.Create(sizeof(glm::mat4[2]) * 8, nullptr, GL_DYNAMIC_DRAW); // or GL_STREAM_DRAW ?

  scaledWidth = width / downscaleFactor;
  scaledHeight = height / downscaleFactor;

  for (auto& img : effectImgs) {
    img.noise.Create(scaledWidth, scaledHeight, GL_RG8, GL_NEAREST);
    img.acc.Create(scaledWidth, scaledHeight, GL_RG32F, GL_NEAREST);
  }

  claimImg.Create(scaledWidth, scaledHeight, GL_R32UI, GL_NEAREST);

  ClearBuffers();
}

void Effect::Destroy() {
  modelSSB.Destroy();
  scrollShader.Destroy();
  fillShader.Destroy();

  for (auto& img : effectImgs) {
    img.noise.Destroy();
    img.acc.Destroy();
  }
  claimImg.Destroy();
}

Texture Effect::Apply(const EffectInputData& in, float dt) {
  std::swap(curr, prev);

  ScatterPass(in, dt);
  FillPass(in);

  return !showAcc ? effectImgs[curr].noise : effectImgs[curr].acc;
}

void Effect::ScatterPass(const EffectInputData& in, float dt) {
  if (accResetInterval > 0) {
    static unsigned frameCount = 0;
    if (++frameCount % accResetInterval == 0) effectImgs[prev].acc.Clear();
  }

  claimImg.Clear({-1, 0, 0, 0});

  scrollShader.Use();

  effectImgs[curr].noise.Bind(0, GL_WRITE_ONLY);
  effectImgs[prev].noise.Bind(1, GL_READ_ONLY);
  effectImgs[curr].acc.Bind(2, GL_WRITE_ONLY);
  effectImgs[prev].acc.Bind(3, GL_READ_WRITE);
  claimImg.Bind(4, GL_READ_WRITE);

  in.currIdTex.Bind(0);

  (in.reproject ? in.prevIdTex : in.currIdTex).Bind(1);

  if (in.reproject) {
    in.prevDepthTex.Bind(2);

    modelSSB.Upload(in.modelMats);
    modelSSB.Bind(0);

    scrollShader.SetMat4v("uViewMat", 2, in.prevCurrView);
    scrollShader.SetMat4v("uProjMat", 2, in.prevCurrProj);
    scrollShader.SetInt("uCurrInd", in.currInd);
  }

  if (in.flow) {
    in.prevFlowTex.Bind(3);

    float speed = scrollSpeedFactor * dt / downscaleFactor * (int)!paused;
    scrollShader.SetFloat("uScrollSpeed", speed);
  }

  scrollShader.SetInt("uReproject", in.reproject);
  scrollShader.SetInt("uFlow", in.flow);

  scrollShader.DispatchCompute(scaledWidth, scaledHeight, 16);
}

void Effect::FillPass(const EffectInputData& in) {
  fillShader.Use();

  effectImgs[curr].noise.Bind(0, GL_READ_WRITE);
  effectImgs[prev].noise.Bind(1, GL_WRITE_ONLY);
  effectImgs[curr].acc.Bind(2, GL_READ_WRITE);
  effectImgs[prev].acc.Bind(3, GL_READ_WRITE);

  in.currIdTex.Bind(0);
  in.prevIdTex.Bind(1);

  fillShader.SetUint("uSeed", util::RandomInt());

  fillShader.DispatchCompute(scaledWidth, scaledHeight, 16);
}

void Effect::ClearBuffers() {
  for (auto& img : effectImgs) {
    img.noise.Clear();
    img.acc.Clear();
  }
}

void Effect::ClearAcc() {
  effectImgs[curr].acc.Clear();
}

void Effect::OnResize(int width, int height) {
  scaledWidth = width / downscaleFactor;
  scaledHeight = height / downscaleFactor;

  for (auto& img : effectImgs) {
    img.noise.Resize(scaledWidth, scaledHeight);
    img.acc.Resize(scaledWidth, scaledHeight);
  }
  claimImg.Resize(scaledWidth, scaledHeight);

  ClearBuffers();
}
