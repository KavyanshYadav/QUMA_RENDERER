#include "MeshRenderEngine.hpp"

#if __has_include(<glad/glad.h>)
#include <glad/glad.h>
#elif __has_include(<glad/gl.h>)
#include <glad/gl.h>
#else
#error "GLAD headers not found. Provide third_party/glad or a glad package."
#endif

#include <array>
#include <cmath>
#include <cstddef>
#include <stdexcept>
#include <string>

namespace sample::rendering {
namespace {

struct Mat4 {
  std::array<float, 16> value{};
};

[[nodiscard]] Mat4 identity() {
  Mat4 matrix{};
  matrix.value = {1.0f, 0.0f, 0.0f, 0.0f,
                  0.0f, 1.0f, 0.0f, 0.0f,
                  0.0f, 0.0f, 1.0f, 0.0f,
                  0.0f, 0.0f, 0.0f, 1.0f};
  return matrix;
}

[[nodiscard]] Mat4 multiply(const Mat4& left, const Mat4& right) {
  Mat4 out{};
  for (int row = 0; row < 4; ++row) {
    for (int col = 0; col < 4; ++col) {
      float sum = 0.0f;
      for (int k = 0; k < 4; ++k) {
        sum += left.value[k * 4 + row] * right.value[col * 4 + k];
      }
      out.value[col * 4 + row] = sum;
    }
  }
  return out;
}

[[nodiscard]] Mat4 perspective(const float fovRadians, const float aspectRatio, const float nearPlane, const float farPlane) {
  Mat4 matrix{};
  const float tanHalf = std::tan(fovRadians * 0.5f);
  matrix.value = {1.0f / (aspectRatio * tanHalf), 0.0f, 0.0f, 0.0f,
                  0.0f, 1.0f / tanHalf, 0.0f, 0.0f,
                  0.0f, 0.0f, -(farPlane + nearPlane) / (farPlane - nearPlane), -1.0f,
                  0.0f, 0.0f, -(2.0f * farPlane * nearPlane) / (farPlane - nearPlane), 0.0f};
  return matrix;
}

[[nodiscard]] Mat4 translate(const float x, const float y, const float z) {
  Mat4 matrix = identity();
  matrix.value[12] = x;
  matrix.value[13] = y;
  matrix.value[14] = z;
  return matrix;
}

[[nodiscard]] Mat4 rotateY(const float radians) {
  Mat4 matrix = identity();
  const float c = std::cos(radians);
  const float s = std::sin(radians);
  matrix.value[0] = c;
  matrix.value[2] = -s;
  matrix.value[8] = s;
  matrix.value[10] = c;
  return matrix;
}

[[nodiscard]] unsigned int compileShader(const unsigned int type, const char* source) {
  const unsigned int shader = glCreateShader(type);
  glShaderSource(shader, 1, &source, nullptr);
  glCompileShader(shader);

  int success = GL_FALSE;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (success != GL_TRUE) {
    int logLength = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
    std::string log(static_cast<std::size_t>(logLength), '\0');
    glGetShaderInfoLog(shader, logLength, nullptr, log.data());
    glDeleteShader(shader);
    throw std::runtime_error("Shader compilation failed: " + log);
  }

  return shader;
}

[[nodiscard]] unsigned int createProgram(const char* vertexSource, const char* fragmentSource) {
  const unsigned int vertex = compileShader(GL_VERTEX_SHADER, vertexSource);
  const unsigned int fragment = compileShader(GL_FRAGMENT_SHADER, fragmentSource);

  const unsigned int program = glCreateProgram();
  glAttachShader(program, vertex);
  glAttachShader(program, fragment);
  glLinkProgram(program);

  int success = GL_FALSE;
  glGetProgramiv(program, GL_LINK_STATUS, &success);
  glDeleteShader(vertex);
  glDeleteShader(fragment);

  if (success != GL_TRUE) {
    int logLength = 0;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
    std::string log(static_cast<std::size_t>(logLength), '\0');
    glGetProgramInfoLog(program, logLength, nullptr, log.data());
    glDeleteProgram(program);
    throw std::runtime_error("Program link failed: " + log);
  }

  return program;
}

constexpr const char* kVertexShader = R"(
#version 330 core
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aUv;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;

out vec3 vNormal;
out vec3 vWorldPos;
out vec2 vUv;

void main() {
  vec4 worldPos = uModel * vec4(aPosition, 1.0);
  vWorldPos = worldPos.xyz;
  vNormal = mat3(transpose(inverse(uModel))) * aNormal;
  vUv = aUv;
  gl_Position = uProjection * uView * worldPos;
}
)";

constexpr const char* kFragmentShader = R"(
#version 330 core
in vec3 vNormal;
in vec3 vWorldPos;
in vec2 vUv;
out vec4 outColor;

uniform vec3 uBaseColor;
uniform vec3 uLightColor;
uniform vec3 uLightPos;
uniform vec3 uCameraPos;
uniform float uAmbient;
uniform float uMetallic;
uniform float uRoughness;
uniform float uAo;

void main() {
  vec3 norm = normalize(vNormal);
  vec3 lightDir = normalize(uLightPos - vWorldPos);
  float diff = max(dot(norm, lightDir), 0.0);

  vec3 viewDir = normalize(uCameraPos - vWorldPos);
  vec3 halfDir = normalize(lightDir + viewDir);
  float smoothness = 1.0 - clamp(uRoughness, 0.04, 1.0);
  float specPower = mix(8.0, 128.0, smoothness);
  float spec = pow(max(dot(norm, halfDir), 0.0), specPower);

  vec3 dielectricF0 = vec3(0.04);
  vec3 f0 = mix(dielectricF0, uBaseColor, clamp(uMetallic, 0.0, 1.0));
  vec3 ambient = uAmbient * uAo * uLightColor;
  vec3 diffuse = diff * uLightColor * (1.0 - clamp(uMetallic, 0.0, 1.0));
  vec3 specular = spec * f0 * uLightColor;

  vec3 lit = (ambient + diffuse + specular) * uBaseColor;
  outColor = vec4(lit, 1.0);
}
)";

} // namespace

MeshRenderEngine::MeshRenderEngine(SDL_Window* window, SDL_GLContext glContext)
    : window_(window), glContext_(glContext) {
  if (window_ == nullptr || glContext_ == nullptr) {
    throw std::runtime_error("MeshRenderEngine requires a valid SDL window and GL context");
  }

  program_ = createProgram(kVertexShader, kFragmentShader);
  glGenVertexArrays(1, &vao_);
  glGenBuffers(1, &vbo_);
  glGenBuffers(1, &ebo_);
  glEnable(GL_DEPTH_TEST);
}

MeshRenderEngine::~MeshRenderEngine() {
  if (ebo_ != 0) {
    glDeleteBuffers(1, &ebo_);
  }
  if (vbo_ != 0) {
    glDeleteBuffers(1, &vbo_);
  }
  if (vao_ != 0) {
    glDeleteVertexArrays(1, &vao_);
  }
  if (program_ != 0) {
    glDeleteProgram(program_);
  }
}

void MeshRenderEngine::setMesh(const MeshData& meshData) {
  mesh_ = meshData;
  indexCount_ = static_cast<std::uint32_t>(mesh_.indices.size());

  glBindVertexArray(vao_);

  glBindBuffer(GL_ARRAY_BUFFER, vbo_);
  glBufferData(GL_ARRAY_BUFFER,
               static_cast<GLsizeiptr>(mesh_.vertices.size() * sizeof(Vertex)),
               mesh_.vertices.data(),
               GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
               static_cast<GLsizeiptr>(mesh_.indices.size() * sizeof(std::uint32_t)),
               mesh_.indices.data(),
               GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, position)));
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, normal)));
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, uv)));
  glEnableVertexAttribArray(2);
}

void MeshRenderEngine::resize(const int drawableWidth, const int drawableHeight) const {
  glViewport(0, 0, drawableWidth, drawableHeight);
}

void MeshRenderEngine::beginFrame(const float clearR, const float clearG, const float clearB) const {
  glClearColor(clearR, clearG, clearB, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void MeshRenderEngine::renderMesh(const CameraState& camera, const SceneLighting& lighting, const float rotationRadians) const {
  if (indexCount_ == 0) {
    return;
  }

  int drawableWidth = 1;
  int drawableHeight = 1;
  SDL_GL_GetDrawableSize(window_, &drawableWidth, &drawableHeight);
  const float aspect = drawableHeight > 0 ? static_cast<float>(drawableWidth) / static_cast<float>(drawableHeight) : 16.0f / 9.0f;

  const Mat4 model = rotateY(rotationRadians);
  const Mat4 view = translate(-camera.position[0], -camera.position[1], -camera.position[2]);
  const Mat4 projection = perspective(camera.fovDegrees * 0.0174532925f, aspect, camera.nearPlane, camera.farPlane);

  glUseProgram(program_);
  glUniformMatrix4fv(glGetUniformLocation(program_, "uModel"), 1, GL_FALSE, model.value.data());
  glUniformMatrix4fv(glGetUniformLocation(program_, "uView"), 1, GL_FALSE, view.value.data());
  glUniformMatrix4fv(glGetUniformLocation(program_, "uProjection"), 1, GL_FALSE, projection.value.data());

  glUniform3f(glGetUniformLocation(program_, "uBaseColor"),
              mesh_.material.baseColor[0],
              mesh_.material.baseColor[1],
              mesh_.material.baseColor[2]);

  glUniform3f(glGetUniformLocation(program_, "uLightColor"),
              lighting.lightColor[0],
              lighting.lightColor[1],
              lighting.lightColor[2]);
  glUniform3f(glGetUniformLocation(program_, "uLightPos"),
              lighting.lightPosition[0],
              lighting.lightPosition[1],
              lighting.lightPosition[2]);
  glUniform3f(glGetUniformLocation(program_, "uCameraPos"),
              camera.position[0],
              camera.position[1],
              camera.position[2]);

  glUniform1f(glGetUniformLocation(program_, "uAmbient"), lighting.ambientIntensity);
  glUniform1f(glGetUniformLocation(program_, "uMetallic"), mesh_.material.metallic);
  glUniform1f(glGetUniformLocation(program_, "uRoughness"), mesh_.material.roughness);
  glUniform1f(glGetUniformLocation(program_, "uAo"), mesh_.material.ambientOcclusion);

  glBindVertexArray(vao_);
  glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indexCount_), GL_UNSIGNED_INT, nullptr);
}

void MeshRenderEngine::endFrame() const {
  SDL_GL_SwapWindow(window_);
}

std::uint32_t MeshRenderEngine::triangleCount() const {
  return indexCount_ / 3;
}

} // namespace sample::rendering
