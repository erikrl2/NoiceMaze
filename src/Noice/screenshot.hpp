#pragma once
#include "image.hpp"
#include "shader.hpp"

#include <string>

class Screenshot {
public:
  enum class Method { Average = 0, AbsDiffSum = 1, Count };

  struct Options {
    Method method = Method::AbsDiffSum;
    int targetFrames = 30;
    float gain = 1.0f;
    float gamma = 1.0f;
    std::string baseName = "capture";
  };

  Options options;

public:
  void Init(int width, int height);
  void Destroy();

  void Update(Texture source);
  void Begin();
  void Reset();
  void SavePNG();

  bool IsActive() const { return hasResult || capturing; }
  bool HasResult() const { return hasResult; }
  Texture GetResult() const { return outImg; }

private:
  void Accumulate(Texture source);
  void Finalize();
  void ClearBuffers();
  void ResizeBuffers(int width, int height);

private:
  int width = 0, height = 0;

  bool capturing = false;
  bool hasResult = false;

  int collectedFrames = 0;

  Shader accumShader;
  Shader finalizeShader;

  Image accumImg;
  Image prevImg;
  Image outImg;
};
