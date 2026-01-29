#pragma once
#include "image.hpp"
#include "shader.hpp"

#include <span>

struct EffectInputData {
  Texture currIdTex;
  Texture prevIdTex;
  Texture prevFlowTex;
  Texture prevDepthTex;
  const glm::mat4* prevCurrProj = nullptr;
  const glm::mat4* prevCurrView = nullptr;
  std::span<const glm::mat4[2]> modelMats;
  int currInd = 0; // 0 or 1
};

class Effect {
public:
  float scrollSpeed = 0.4f;
  int accResetInterval = 10;
  bool showAcc = false;

public:
  void Init(int width, int height);
  void Destroy();

  Texture Apply(const EffectInputData& in, float dt);

  void OnResize(int width, int height);

private:
  void Clear();
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
