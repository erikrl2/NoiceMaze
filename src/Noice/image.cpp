#include "image.hpp"

#include <cassert>

struct FormatInfo {
  GLenum format;
  GLenum type;
};

namespace util {
  FormatInfo GetFormatInfo(GLenum internalFormat);
}

void Texture::CreateOrResize(int width, int height, GLint internalFormat, GLint filter) {
  Destroy();
  FormatInfo formatInfo = util::GetFormatInfo(internalFormat);
  glGenTextures(1, &id);
  glBindTexture(GL_TEXTURE_2D, id);
  glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, formatInfo.format, formatInfo.type, nullptr);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
  // glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::Destroy() {
  if (id) {
    glDeleteTextures(1, &id);
    id = 0;
  }
}

void Texture::Bind(unsigned unit) const {
  glActiveTexture(GL_TEXTURE0 + unit);
  glBindTexture(GL_TEXTURE_2D, id);
}

void Image::Create(int width, int height, GLint internalFormat, GLint filter) {
  assert(!tex);
  this->width = width;
  this->height = height;
  this->internalFormat = internalFormat;
  this->filter = filter;
  tex.CreateOrResize(width, height, internalFormat, filter);
}

void Image::Resize(int width, int height) {
  if (this->width == width && this->height == height) return;
  assert(tex);
  Destroy();
  Create(width, height, internalFormat, filter);
}

void Image::Destroy() {
  tex.Destroy();
}

void Image::Bind(unsigned unit, GLenum access) const {
  glBindImageTexture(unit, tex.id, 0, GL_FALSE, 0, access, internalFormat);
}

void Image::Clear(const glm::vec4& color) const {
  FormatInfo info = util::GetFormatInfo(internalFormat);

  const bool isInteger = info.format == GL_RED_INTEGER || info.format == GL_RG_INTEGER || info.format == GL_RGB_INTEGER
      || info.format == GL_RGBA_INTEGER;

  if (!isInteger) {
    glClearTexImage(tex.id, 0, info.format, GL_FLOAT, &color[0]);
  } else {
    switch (info.type) {
    case GL_UNSIGNED_BYTE:
    case GL_UNSIGNED_SHORT:
    case GL_UNSIGNED_INT: {
      glm::uvec4 v = glm::ivec4(color); // (float->int->uint)
      glClearTexImage(tex.id, 0, info.format, GL_UNSIGNED_INT, &v[0]);
      break;
    }
    case GL_BYTE:
    case GL_SHORT:
    case GL_INT: {
      glm::ivec4 v = color;
      glClearTexImage(tex.id, 0, info.format, GL_INT, &v[0]);
      break;
    }
    default: assert(false);
    }
  }
}

void Image::Upload(unsigned char* data) const {
  glBindTexture(GL_TEXTURE_2D, tex.id);
  FormatInfo info = util::GetFormatInfo(internalFormat);
  glPixelStorei(GL_UNPACK_ALIGNMENT, (info.format == GL_RGBA) ? 4 : 1);
  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, info.format, info.type, data);
}

std::vector<unsigned char> Image::Download() const {
  std::vector<unsigned char> data(width * height * 4);
  glBindTexture(GL_TEXTURE_2D, tex.id);
  FormatInfo info = util::GetFormatInfo(internalFormat);
  glGetTexImage(GL_TEXTURE_2D, 0, info.format, info.type, data.data());
  return data;
}

namespace util {
  FormatInfo GetFormatInfo(GLenum internalFormat) {
    switch (internalFormat) {
    case GL_R16F: return {GL_RED, GL_HALF_FLOAT};
    case GL_RG16F: return {GL_RG, GL_HALF_FLOAT};
    case GL_RGB16F: return {GL_RGB, GL_HALF_FLOAT};
    case GL_RGBA16F: return {GL_RGBA, GL_HALF_FLOAT};

    case GL_R32F: return {GL_RED, GL_FLOAT};
    case GL_RG32F: return {GL_RG, GL_FLOAT};
    case GL_RGB32F: return {GL_RGB, GL_FLOAT};
    case GL_RGBA32F: return {GL_RGBA, GL_FLOAT};

    case GL_R8: return {GL_RED, GL_UNSIGNED_BYTE};
    case GL_RG8: return {GL_RG, GL_UNSIGNED_BYTE};
    case GL_RGB8: return {GL_RGB, GL_UNSIGNED_BYTE};
    case GL_RGBA8: return {GL_RGBA, GL_UNSIGNED_BYTE};

    case GL_R16: return {GL_RED, GL_UNSIGNED_SHORT};
    case GL_RG16: return {GL_RG, GL_UNSIGNED_SHORT};
    case GL_RGB16: return {GL_RGB, GL_UNSIGNED_SHORT};
    case GL_RGBA16: return {GL_RGBA, GL_UNSIGNED_SHORT};

    case GL_R8I: return {GL_RED_INTEGER, GL_BYTE};
    case GL_RG8I: return {GL_RG_INTEGER, GL_BYTE};
    case GL_RGB8I: return {GL_RGB_INTEGER, GL_BYTE};
    case GL_RGBA8I: return {GL_RGBA_INTEGER, GL_BYTE};

    case GL_R16I: return {GL_RED_INTEGER, GL_SHORT};
    case GL_RG16I: return {GL_RG_INTEGER, GL_SHORT};
    case GL_RGB16I: return {GL_RGB_INTEGER, GL_SHORT};
    case GL_RGBA16I: return {GL_RGBA_INTEGER, GL_SHORT};

    case GL_R32I: return {GL_RED_INTEGER, GL_INT};
    case GL_RG32I: return {GL_RG_INTEGER, GL_INT};
    case GL_RGB32I: return {GL_RGB_INTEGER, GL_INT};
    case GL_RGBA32I: return {GL_RGBA_INTEGER, GL_INT};

    case GL_R8UI: return {GL_RED_INTEGER, GL_UNSIGNED_BYTE};
    case GL_RG8UI: return {GL_RG_INTEGER, GL_UNSIGNED_BYTE};
    case GL_RGB8UI: return {GL_RGB_INTEGER, GL_UNSIGNED_BYTE};
    case GL_RGBA8UI: return {GL_RGBA_INTEGER, GL_UNSIGNED_BYTE};

    case GL_R16UI: return {GL_RED_INTEGER, GL_UNSIGNED_SHORT};
    case GL_RG16UI: return {GL_RG_INTEGER, GL_UNSIGNED_SHORT};
    case GL_RGB16UI: return {GL_RGB_INTEGER, GL_UNSIGNED_SHORT};
    case GL_RGBA16UI: return {GL_RGBA_INTEGER, GL_UNSIGNED_SHORT};

    case GL_R32UI: return {GL_RED_INTEGER, GL_UNSIGNED_INT};
    case GL_RG32UI: return {GL_RG_INTEGER, GL_UNSIGNED_INT};
    case GL_RGB32UI: return {GL_RGB_INTEGER, GL_UNSIGNED_INT};
    case GL_RGBA32UI: return {GL_RGBA_INTEGER, GL_UNSIGNED_INT};

    case GL_DEPTH_COMPONENT16: return {GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT};
    case GL_DEPTH_COMPONENT24: return {GL_DEPTH_COMPONENT, GL_UNSIGNED_INT};
    case GL_DEPTH_COMPONENT32F: return {GL_DEPTH_COMPONENT, GL_FLOAT};

    case GL_DEPTH24_STENCIL8: return {GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8};
    case GL_STENCIL_INDEX8: return {GL_STENCIL_INDEX, GL_UNSIGNED_BYTE};

    default: assert(false); return {GL_RGBA, GL_UNSIGNED_BYTE}; // Fallback
    }
  }
} // namespace util
