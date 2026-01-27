#include "shader.hpp"

#include <fstream>
#include <iostream>
#include <sstream>

namespace util {
  void printShaderLog(GLuint shader, const char* name);
  void printProgramLog(GLuint prog, const char* name);

  std::string ReadFileString(const char* filepath);
} // namespace util

void Shader::CreateCompute(const char* s) {
  Destroy();

  std::string cs = util::ReadFileString(s);

  const char* cc = cs.c_str();

  unsigned int comp = glCreateShader(GL_COMPUTE_SHADER);
  glShaderSource(comp, 1, &cc, nullptr);
  glCompileShader(comp);
  util::printShaderLog(comp, s);

  program = glCreateProgram();
  glAttachShader(program, comp);
  glLinkProgram(program);
  util::printProgramLog(program, s);

  glDeleteShader(comp);
}

void Shader::Destroy() {
  if (program) {
    glDeleteProgram(program);
    program = 0;
  }
  uniformLocationCache.clear();
}

void Shader::Use() const {
  glUseProgram(program);
}

GLint Shader::GetUniformLocation(const std::string& name) const {
  if (auto it = uniformLocationCache.find(name); it != uniformLocationCache.end()) return it->second;
  GLint location = glGetUniformLocation(program, name.c_str());
  uniformLocationCache.emplace(name, location);
  return location;
}

void Shader::SetInt(const std::string& name, int value) const {
  glUniform1i(GetUniformLocation(name), value);
}

void Shader::SetUint(const std::string& name, unsigned int value) const {
  glUniform1ui(GetUniformLocation(name), value);
}

void Shader::SetFloat(const std::string& name, float value) const {
  glUniform1f(GetUniformLocation(name), value);
}

void Shader::SetVec2(const std::string& name, const glm::vec2& v) const {
  glUniform2f(GetUniformLocation(name), v.x, v.y);
}

void Shader::SetVec3(const std::string& name, const glm::vec3& v) const {
  glUniform3f(GetUniformLocation(name), v.x, v.y, v.z);
}

void Shader::SetMat4(const std::string& name, const glm::mat4& m) const {
  glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, &m[0][0]);
}

void Shader::SetMat4v(const std::string& name, GLsizei count, const glm::mat4* m) const {
  glUniformMatrix4fv(GetUniformLocation(name), count, GL_FALSE, &m[0][0][0]);
}

void Shader::DispatchCompute(int width, int height, int groupSize, bool barrier) const {
  int numGroupsX = (width + groupSize - 1) / groupSize;
  int numGroupsY = (height + groupSize - 1) / groupSize;
  glDispatchCompute(numGroupsX, numGroupsY, 1);
  if (barrier) glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);
}

// ---

void StorageBuffer::Create(GLsizeiptr newSizeBytes, const void* initialData, GLenum newUsage) {
  Destroy();

  usage = newUsage;
  sizeBytes = newSizeBytes;

  glGenBuffers(1, &id);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, id);
  glBufferData(GL_SHADER_STORAGE_BUFFER, sizeBytes, initialData, usage);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void StorageBuffer::Destroy() {
  if (id) {
    glDeleteBuffers(1, &id);
    id = 0;
  }
}

void StorageBuffer::Upload(GLsizeiptr newSizeBytes, const void* data) {
  if (!id) {
    Create(newSizeBytes, data, usage);
  }
  if (newSizeBytes > sizeBytes) {
    GLsizeiptr newCapacity = sizeBytes > 0 ? sizeBytes : 256;
    while (newCapacity < newSizeBytes) newCapacity *= 2;
    Create(newCapacity, nullptr, usage);
  }
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, id);
  glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, newSizeBytes, data);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void StorageBuffer::Bind(GLuint binding) const {
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding, id);
}


namespace util {
  void printShaderLog(GLuint shader, const char* name) {
    GLint ok = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (ok == GL_FALSE) {
      GLint len = 0;
      glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
      std::vector<char> buf(len ? len : 1);
      glGetShaderInfoLog(shader, (GLsizei)buf.size(), nullptr, buf.data());
      std::cerr << "Shader compile error (" << (name ? name : "") << "):\n" << buf.data() << "\n";
    }
  }

  void printProgramLog(GLuint prog, const char* name) {
    GLint ok = 0;
    glGetProgramiv(prog, GL_LINK_STATUS, &ok);
    if (ok == GL_FALSE) {
      GLint len = 0;
      glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &len);
      std::vector<char> buf(len ? len : 1);
      glGetProgramInfoLog(prog, (GLsizei)buf.size(), nullptr, buf.data());
      std::cerr << "Program link error (" << (name ? name : "") << "):\n" << buf.data() << "\n";
    }
  }

  std::string ReadFileString(const char* path) {
    std::ifstream f(path);
    std::stringstream ss;
    ss << f.rdbuf();
    return ss.str();
  }
} // namespace util
