#pragma once

#include <SDL.h>

#include <cstdint>

#include "ObjLoader.hpp"

namespace sample::rendering {

struct SceneLighting {
  float lightPosition[3]{2.5f, 4.0f, 2.5f};
  float lightColor[3]{1.0f, 1.0f, 1.0f};
  float ambientIntensity = 0.18f;
};

struct CameraState {
  float position[3]{0.0f, 0.0f, 5.0f};
  float fovDegrees = 45.0f;
  float nearPlane = 0.1f;
  float farPlane = 100.0f;
};

class MeshRenderEngine {
public:
  MeshRenderEngine(SDL_Window* window, SDL_GLContext glContext);
  ~MeshRenderEngine();

  MeshRenderEngine(const MeshRenderEngine&) = delete;
  MeshRenderEngine& operator=(const MeshRenderEngine&) = delete;

  void setMesh(const MeshData& meshData);
  void resize(int drawableWidth, int drawableHeight) const;

  void beginFrame(const float clearR, const float clearG, const float clearB) const;
  void renderMesh(const CameraState& camera, const SceneLighting& lighting, float rotationRadians) const;
  void endFrame() const;

  [[nodiscard]] std::uint32_t triangleCount() const;

private:
  SDL_Window* window_ = nullptr;
  SDL_GLContext glContext_ = nullptr;

  unsigned int program_ = 0;
  unsigned int vao_ = 0;
  unsigned int vbo_ = 0;
  unsigned int ebo_ = 0;
  std::uint32_t indexCount_ = 0;

  MeshData mesh_{};
};

} // namespace sample::rendering
