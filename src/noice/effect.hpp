#pragma once
#include "image.hpp"
#include "shader.hpp"

#include <span>

struct EffectInputData {
  Texture currIdTex;
  Texture prevIdTex;
  Texture prevDepthTex;
  const glm::mat4* prevCurrProj = nullptr; // [1-currInd]: prev, [currInd]: curr
  const glm::mat4* prevCurrView = nullptr; // [1-currInd]: prev, [currInd]: curr
  std::span<const glm::mat4[2]> modelMats;
  int currInd = 0; // 0 or 1
};

class Effect {
public:
  int accResetInterval = 0;
  bool showAcc = false;

public:
  void Init(int width, int height);
  void Destroy();

  Texture Apply(const EffectInputData& in, float dt);

  void OnResize(int width, int height);

private:
  void ScatterPass(const EffectInputData& in, float dt);
  void FillPass(const EffectInputData& in);

  void ResetBuffers();

private:
  int width = 0, height = 0;

  struct EffectImage {
    Image noise;
    Image acc;
  };

  EffectImage effectImgs[2];
  int curr = 0, prev = 1;

  Image claimImg;

  Shader scrollShader;
  Shader fillShader;

  StorageBuffer modelSSB;
};
