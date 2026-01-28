#include "screenshot.hpp"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#include <iostream>

void Screenshot::Init(int width, int height) {
  accumShader.CreateCompute("Shader/Noice/screenshot_accum.comp.glsl");
  finalizeShader.CreateCompute("Shader/Noice/screenshot_finalize.comp.glsl");

  accumImg.Create(width, height, GL_R16F, GL_NEAREST);
  prevImg.Create(width, height, GL_R8, GL_NEAREST);
  outImg.Create(width, height, GL_R8, GL_NEAREST);

  this->width = width;
  this->height = height;
}

void Screenshot::Destroy() {
  accumShader.Destroy();
  finalizeShader.Destroy();

  accumImg.Destroy();
  prevImg.Destroy();
  outImg.Destroy();
}

void Screenshot::Update(Texture source) {
  if (!capturing) return;

  Accumulate(source);
  collectedFrames++;

  Finalize();

  if (collectedFrames >= options.targetFrames) {
    capturing = false;
    hasResult = true;
  }
}

void Screenshot::Accumulate(Texture source) {
  accumShader.Use();

  accumImg.Bind(0, GL_READ_WRITE);
  prevImg.Bind(1, GL_READ_WRITE);

  source.Bind(0);

  accumShader.SetInt("uMethod", (int)options.method);
  accumShader.SetInt("uFrameIndex", collectedFrames);

  accumShader.DispatchCompute(width, height, 16);
}

void Screenshot::Finalize() {
  finalizeShader.Use();

  accumImg.Bind(0, GL_READ_ONLY);
  outImg.Bind(1, GL_WRITE_ONLY);

  finalizeShader.SetInt("uMethod", (int)options.method);
  finalizeShader.SetInt("uFrames", collectedFrames);
  finalizeShader.SetFloat("uGain", options.gain);
  finalizeShader.SetFloat("uGamma", options.gamma);

  finalizeShader.DispatchCompute(width, height, 16);
}

void Screenshot::Begin() {
  capturing = true;
  hasResult = false;
  collectedFrames = 0;

  ClearBuffers();
}

void Screenshot::Reset() {
  capturing = false;
  hasResult = false;
  collectedFrames = 0;
}

void Screenshot::ClearBuffers() {
  accumImg.Clear();
  prevImg.Clear();
  outImg.Clear();
}

void Screenshot::ResizeBuffers(int width, int height) {
  this->width = width, this->height = height;

  accumImg.Resize(width, height);
  prevImg.Resize(width, height);
  outImg.Resize(width, height);

  Reset();
}

void Screenshot::SavePNG() {
  if (!hasResult) return;

  std::vector<unsigned char> pixels = outImg.Download();

  std::string filename = options.baseName + ".png";

  stbi_flip_vertically_on_write(1);
  int ok = stbi_write_png(filename.c_str(), outImg.GetWidth(), outImg.GetHeight(), 1, pixels.data(), outImg.GetWidth());

  if (!ok) {
    std::cerr << "Screenshot: failed to write png: " << filename << "\n";
  } else {
    std::cout << "Screenshot: saved " << filename << " (" << outImg.GetHeight() << "x" << outImg.GetHeight() << ")\n";
  }
}
