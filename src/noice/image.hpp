#pragma once
#include <GL/glew.h>
#include <glm/glm.hpp>

#include <vector>

class Image;

struct Texture {
  GLuint id = 0;

  void Bind(unsigned unit) const;

  operator GLuint() const { return id; }
  operator bool() const { return id != 0; }

private:
  void CreateOrResize(int width, int height, GLint internalFormat, GLint filter);
  void Destroy();

  friend class Image;
};

class Image {
public:
  void Create(int width, int height, GLint internalFormat, GLint filter);
  void Resize(int width, int height);
  void Destroy();

  Texture GetTexture() const { return tex; }
  int GetWidth() const { return width; }
  int GetHeight() const { return height; }

  void Bind(unsigned unit, GLenum access) const;
  void Clear(const glm::vec4& color = {}) const;
  void Upload(unsigned char* data) const;
  std::vector<unsigned char> Download() const;

  operator Texture() const { return tex; }
  operator bool() const { return tex.id != 0; }

  ~Image() { assert(!*this); }

private:
  Texture tex;
  int width = 0;
  int height = 0;
  GLint internalFormat = GL_RGBA8;
  GLint filter = GL_LINEAR;
};
