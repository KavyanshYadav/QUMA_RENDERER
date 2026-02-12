#include "MeshRenderEngine.hpp"

#if __has_include(<glad/glad.h>)
#include <glad/glad.h>
#elif __has_include(<glad/gl.h>)
#include <glad/gl.h>
#else
#error "GLAD headers not found. Provide third_party/glad or a glad package."
#endif

#include <SDL.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <string>

namespace sample::rendering {
namespace {

struct Vec3 {
  float x = 0.0f;
  float y = 0.0f;
  float z = 0.0f;
};

struct Mat4 {
  std::array<float, 16> value{};
};

[[nodiscard]] Vec3 operator+(const Vec3& a, const Vec3& b) { return {a.x + b.x, a.y + b.y, a.z + b.z}; }
[[nodiscard]] Vec3 operator-(const Vec3& a, const Vec3& b) { return {a.x - b.x, a.y - b.y, a.z - b.z}; }
[[nodiscard]] Vec3 operator*(const Vec3& a, const float scalar) { return {a.x * scalar, a.y * scalar, a.z * scalar}; }

[[nodiscard]] float dot(const Vec3& a, const Vec3& b) { return a.x * b.x + a.y * b.y + a.z * b.z; }

[[nodiscard]] Vec3 cross(const Vec3& a, const Vec3& b) {
  return {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
}

[[nodiscard]] float length(const Vec3& v) { return std::sqrt(dot(v, v)); }

[[nodiscard]] Vec3 normalize(const Vec3& v) {
  const float len = length(v);
  if (len <= 0.0001f) {
    return {0.0f, 0.0f, -1.0f};
  }
  return v * (1.0f / len);
}

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

[[nodiscard]] Mat4 lookAt(const Vec3& eye, const Vec3& center, const Vec3& upHint) {
  const Vec3 forward = normalize(center - eye);
  const Vec3 right = normalize(cross(forward, upHint));
  const Vec3 up = cross(right, forward);

  Mat4 matrix = identity();
  matrix.value[0] = right.x;
  matrix.value[1] = up.x;
  matrix.value[2] = -forward.x;
  matrix.value[4] = right.y;
  matrix.value[5] = up.y;
  matrix.value[6] = -forward.y;
  matrix.value[8] = right.z;
  matrix.value[9] = up.z;
  matrix.value[10] = -forward.z;
  matrix.value[12] = -dot(right, eye);
  matrix.value[13] = -dot(up, eye);
  matrix.value[14] = dot(forward, eye);
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

[[nodiscard]] Mat4 scaleUniform(const float scale) {
  Mat4 matrix = identity();
  matrix.value[0] = scale;
  matrix.value[5] = scale;
  matrix.value[10] = scale;
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

[[nodiscard]] bool rayIntersectsAabb(const Vec3& rayOrigin,
                                     const Vec3& rayDirection,
                                     const Vec3& minBounds,
                                     const Vec3& maxBounds,
                                     float& outDistance) {
  float tMin = 0.0f;
  float tMax = std::numeric_limits<float>::max();

  auto axisCheck = [&](const float origin, const float direction, const float minVal, const float maxVal) {
    if (std::abs(direction) < 0.0001f) {
      return origin >= minVal && origin <= maxVal;
    }

    float invD = 1.0f / direction;
    float t0 = (minVal - origin) * invD;
    float t1 = (maxVal - origin) * invD;
    if (t0 > t1) {
      std::swap(t0, t1);
    }
    tMin = std::max(tMin, t0);
    tMax = std::min(tMax, t1);
    return tMax >= tMin;
  };

  if (!axisCheck(rayOrigin.x, rayDirection.x, minBounds.x, maxBounds.x)) {
    return false;
  }
  if (!axisCheck(rayOrigin.y, rayDirection.y, minBounds.y, maxBounds.y)) {
    return false;
  }
  if (!axisCheck(rayOrigin.z, rayDirection.z, minBounds.z, maxBounds.z)) {
    return false;
  }

  outDistance = tMin;
  return true;
}

[[nodiscard]] Vec3 createRayDirectionFromScreen(const int mouseX,
                                                const int mouseY,
                                                const int width,
                                                const int height,
                                                const CameraState& camera) {
  const float xNdc = (2.0f * static_cast<float>(mouseX) / static_cast<float>(width)) - 1.0f;
  const float yNdc = 1.0f - (2.0f * static_cast<float>(mouseY) / static_cast<float>(height));

  const float tanHalf = std::tan(camera.fovDegrees * 0.0174532925f * 0.5f);
  const float aspect = static_cast<float>(width) / static_cast<float>(height);
  Vec3 dirCamera{xNdc * aspect * tanHalf, yNdc * tanHalf, -1.0f};

  const Vec3 forward = normalize({camera.forward[0], camera.forward[1], camera.forward[2]});
  const Vec3 up = normalize({camera.up[0], camera.up[1], camera.up[2]});
  const Vec3 right = normalize(cross(forward, up));

  Vec3 worldDir = normalize((right * dirCamera.x) + (up * dirCamera.y) + (forward * -dirCamera.z));
  return worldDir;
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

void main() {
  vec4 worldPos = uModel * vec4(aPosition, 1.0);
  vWorldPos = worldPos.xyz;
  vNormal = mat3(transpose(inverse(uModel))) * aNormal;
  gl_Position = uProjection * uView * worldPos;
}
)";

constexpr const char* kFragmentShader = R"(
#version 330 core
in vec3 vNormal;
in vec3 vWorldPos;
out vec4 outColor;

uniform vec3 uBaseColor;
uniform vec3 uLightColor;
uniform vec3 uLightPos;
uniform vec3 uCameraPos;
uniform float uAmbient;
uniform float uMetallic;
uniform float uRoughness;
uniform float uAo;
uniform float uHighlight;
uniform float uSelected;

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
  vec3 hoveredTint = mix(lit, vec3(1.0, 0.8, 0.1), uHighlight * 0.35);
  vec3 selectedTint = mix(hoveredTint, vec3(0.2, 1.0, 0.3), uSelected * 0.40);
  outColor = vec4(selectedTint, 1.0);
}
)";

} // namespace

struct MeshRenderEngine::GpuMesh {
  std::uint32_t id = 0;
  MeshData mesh;
  unsigned int vao = 0;
  unsigned int vbo = 0;
  unsigned int ebo = 0;
  std::uint32_t indexCount = 0;
  Vec3 localBoundsMin{};
  Vec3 localBoundsMax{};
  Vec3 position{};
  float rotationYRadians = 0.0f;
  float scale = 1.0f;
};

MeshRenderEngine::MeshRenderEngine(SDL_Window* window)
    : window_(window) {
  if (window_ == nullptr) {
    throw std::runtime_error("MeshRenderEngine requires a valid SDL window");
  }

  program_ = createProgram(kVertexShader, kFragmentShader);
  glEnable(GL_DEPTH_TEST);
}

MeshRenderEngine::~MeshRenderEngine() {
  for (auto& mesh : meshes_) {
    if (mesh.ebo != 0) {
      glDeleteBuffers(1, &mesh.ebo);
    }
    if (mesh.vbo != 0) {
      glDeleteBuffers(1, &mesh.vbo);
    }
    if (mesh.vao != 0) {
      glDeleteVertexArrays(1, &mesh.vao);
    }
  }

  if (program_ != 0) {
    glDeleteProgram(program_);
  }
}

std::uint32_t MeshRenderEngine::addMeshInstance(const MeshInstanceCreateInfo& createInfo) {
  GpuMesh gpuMesh{};
  gpuMesh.id = nextMeshId_++;
  gpuMesh.mesh = createInfo.mesh;
  gpuMesh.position = {createInfo.position[0], createInfo.position[1], createInfo.position[2]};
  gpuMesh.rotationYRadians = createInfo.rotationYRadians;
  gpuMesh.scale = createInfo.scale;
  gpuMesh.indexCount = static_cast<std::uint32_t>(gpuMesh.mesh.indices.size());

  if (gpuMesh.mesh.vertices.empty() || gpuMesh.mesh.indices.empty()) {
    throw std::runtime_error("Cannot add empty mesh instance");
  }

  const auto& first = gpuMesh.mesh.vertices.front();
  gpuMesh.localBoundsMin = {first.position[0], first.position[1], first.position[2]};
  gpuMesh.localBoundsMax = gpuMesh.localBoundsMin;
  for (const auto& vertex : gpuMesh.mesh.vertices) {
    gpuMesh.localBoundsMin.x = std::min(gpuMesh.localBoundsMin.x, vertex.position[0]);
    gpuMesh.localBoundsMin.y = std::min(gpuMesh.localBoundsMin.y, vertex.position[1]);
    gpuMesh.localBoundsMin.z = std::min(gpuMesh.localBoundsMin.z, vertex.position[2]);
    gpuMesh.localBoundsMax.x = std::max(gpuMesh.localBoundsMax.x, vertex.position[0]);
    gpuMesh.localBoundsMax.y = std::max(gpuMesh.localBoundsMax.y, vertex.position[1]);
    gpuMesh.localBoundsMax.z = std::max(gpuMesh.localBoundsMax.z, vertex.position[2]);
  }

  glGenVertexArrays(1, &gpuMesh.vao);
  glGenBuffers(1, &gpuMesh.vbo);
  glGenBuffers(1, &gpuMesh.ebo);

  glBindVertexArray(gpuMesh.vao);
  glBindBuffer(GL_ARRAY_BUFFER, gpuMesh.vbo);
  glBufferData(GL_ARRAY_BUFFER,
               static_cast<GLsizeiptr>(gpuMesh.mesh.vertices.size() * sizeof(Vertex)),
               gpuMesh.mesh.vertices.data(),
               GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gpuMesh.ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
               static_cast<GLsizeiptr>(gpuMesh.mesh.indices.size() * sizeof(std::uint32_t)),
               gpuMesh.mesh.indices.data(),
               GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, position)));
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, normal)));
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, uv)));
  glEnableVertexAttribArray(2);

  const std::uint32_t newId = gpuMesh.id;
  meshes_.push_back(std::move(gpuMesh));
  return newId;
}

void MeshRenderEngine::updateMeshMaterial(const std::uint32_t meshId, const PbrMaterial& material) {
  auto it = std::find_if(meshes_.begin(), meshes_.end(), [meshId](const GpuMesh& mesh) { return mesh.id == meshId; });
  if (it != meshes_.end()) {
    it->mesh.material = material;
  }
}

std::optional<std::uint32_t> MeshRenderEngine::pickMeshFromScreen(const int mouseX, const int mouseY, const CameraState& camera) const {
  int width = 1;
  int height = 1;
  SDL_GL_GetDrawableSize(window_, &width, &height);
  if (width <= 0 || height <= 0) {
    return std::nullopt;
  }

  const Vec3 origin{camera.position[0], camera.position[1], camera.position[2]};
  const Vec3 direction = createRayDirectionFromScreen(mouseX, mouseY, width, height, camera);

  std::optional<std::uint32_t> bestMesh;
  float bestDistance = std::numeric_limits<float>::max();
  for (const auto& mesh : meshes_) {
    const Vec3 minBounds = (mesh.localBoundsMin * mesh.scale) + mesh.position;
    const Vec3 maxBounds = (mesh.localBoundsMax * mesh.scale) + mesh.position;
    float distance = 0.0f;
    if (!rayIntersectsAabb(origin, direction, minBounds, maxBounds, distance)) {
      continue;
    }
    if (distance < bestDistance) {
      bestDistance = distance;
      bestMesh = mesh.id;
    }
  }

  return bestMesh;
}

std::optional<std::uint32_t> MeshRenderEngine::findLookedAtMesh(const CameraState& camera) const {
  int width = 1;
  int height = 1;
  SDL_GL_GetDrawableSize(window_, &width, &height);
  return pickMeshFromScreen(width / 2, height / 2, camera);
}

void MeshRenderEngine::setHoveredMesh(const std::optional<std::uint32_t> meshId) {
  hoveredMeshId_ = meshId;
}

void MeshRenderEngine::setSelectedMesh(const std::optional<std::uint32_t> meshId) {
  selectedMeshId_ = meshId;
}

void MeshRenderEngine::resize(const int drawableWidth, const int drawableHeight) const {
  glViewport(0, 0, drawableWidth, drawableHeight);
}

void MeshRenderEngine::beginFrame(const float clearR, const float clearG, const float clearB) const {
  glClearColor(clearR, clearG, clearB, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void MeshRenderEngine::renderScene(const CameraState& camera, const SceneLighting& lighting) const {
  int drawableWidth = 1;
  int drawableHeight = 1;
  SDL_GL_GetDrawableSize(window_, &drawableWidth, &drawableHeight);
  const float aspect = drawableHeight > 0 ? static_cast<float>(drawableWidth) / static_cast<float>(drawableHeight) : 16.0f / 9.0f;

  const Vec3 eye{camera.position[0], camera.position[1], camera.position[2]};
  const Vec3 forward = normalize({camera.forward[0], camera.forward[1], camera.forward[2]});
  const Vec3 up = normalize({camera.up[0], camera.up[1], camera.up[2]});
  const Mat4 view = lookAt(eye, eye + forward, up);
  const Mat4 projection = perspective(camera.fovDegrees * 0.0174532925f, aspect, camera.nearPlane, camera.farPlane);

  glUseProgram(program_);
  glUniformMatrix4fv(glGetUniformLocation(program_, "uView"), 1, GL_FALSE, view.value.data());
  glUniformMatrix4fv(glGetUniformLocation(program_, "uProjection"), 1, GL_FALSE, projection.value.data());
  glUniform3f(glGetUniformLocation(program_, "uLightColor"), lighting.lightColor[0], lighting.lightColor[1], lighting.lightColor[2]);
  glUniform3f(glGetUniformLocation(program_, "uLightPos"), lighting.lightPosition[0], lighting.lightPosition[1], lighting.lightPosition[2]);
  glUniform3f(glGetUniformLocation(program_, "uCameraPos"), camera.position[0], camera.position[1], camera.position[2]);
  glUniform1f(glGetUniformLocation(program_, "uAmbient"), lighting.ambientIntensity);

  for (const auto& mesh : meshes_) {
    const Mat4 model = multiply(translate(mesh.position.x, mesh.position.y, mesh.position.z),
                                multiply(rotateY(mesh.rotationYRadians), scaleUniform(mesh.scale)));

    glUniformMatrix4fv(glGetUniformLocation(program_, "uModel"), 1, GL_FALSE, model.value.data());
    glUniform3f(glGetUniformLocation(program_, "uBaseColor"),
                mesh.mesh.material.baseColor[0],
                mesh.mesh.material.baseColor[1],
                mesh.mesh.material.baseColor[2]);
    glUniform1f(glGetUniformLocation(program_, "uMetallic"), mesh.mesh.material.metallic);
    glUniform1f(glGetUniformLocation(program_, "uRoughness"), mesh.mesh.material.roughness);
    glUniform1f(glGetUniformLocation(program_, "uAo"), mesh.mesh.material.ambientOcclusion);
    glUniform1f(glGetUniformLocation(program_, "uHighlight"), hoveredMeshId_.has_value() && hoveredMeshId_.value() == mesh.id ? 1.0f : 0.0f);
    glUniform1f(glGetUniformLocation(program_, "uSelected"), selectedMeshId_.has_value() && selectedMeshId_.value() == mesh.id ? 1.0f : 0.0f);

    glBindVertexArray(mesh.vao);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(mesh.indexCount), GL_UNSIGNED_INT, nullptr);
  }
}

void MeshRenderEngine::endFrame() const {
  SDL_GL_SwapWindow(window_);
}

std::uint32_t MeshRenderEngine::totalTriangles() const {
  std::uint32_t total = 0;
  for (const auto& mesh : meshes_) {
    total += mesh.indexCount / 3;
  }
  return total;
}

} // namespace sample::rendering
