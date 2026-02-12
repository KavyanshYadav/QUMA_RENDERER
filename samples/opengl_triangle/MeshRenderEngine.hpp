#pragma once

#include <SDL.h>

#include <cstdint>
#include <optional>
#include <vector>

#include "ObjLoader.hpp"

namespace sample::rendering {

struct SceneLighting {
  float lightPosition[3]{2.5f, 4.0f, 2.5f};
  float lightColor[3]{1.0f, 1.0f, 1.0f};
  float ambientIntensity = 0.18f;
};

struct CameraState {
  float position[3]{0.0f, 0.0f, 5.0f};
  float forward[3]{0.0f, 0.0f, -1.0f};
  float up[3]{0.0f, 1.0f, 0.0f};
  float fovDegrees = 60.0f;
  float nearPlane = 0.1f;
  float farPlane = 150.0f;
};

class MeshRenderEngine {
public:
  struct MeshInstanceCreateInfo {
    MeshData mesh;
    float position[3]{0.0f, 0.0f, 0.0f};
    float rotationYRadians = 0.0f;
    float scale = 1.0f;
  };

  struct MeshTransform {
    float position[3]{0.0f, 0.0f, 0.0f};
    float rotationYRadians = 0.0f;
    float scale = 1.0f;
  };

  explicit MeshRenderEngine(SDL_Window* window);
  ~MeshRenderEngine();

  MeshRenderEngine(const MeshRenderEngine&) = delete;
  MeshRenderEngine& operator=(const MeshRenderEngine&) = delete;

  [[nodiscard]] std::uint32_t addMeshInstance(const MeshInstanceCreateInfo& createInfo);
  void updateMeshMaterial(std::uint32_t meshId, const PbrMaterial& material);

  [[nodiscard]] std::optional<std::uint32_t> pickMeshFromScreen(int mouseX, int mouseY, const CameraState& camera) const;
  [[nodiscard]] std::optional<std::uint32_t> findLookedAtMesh(const CameraState& camera) const;

  void setHoveredMesh(std::optional<std::uint32_t> meshId);
  void setSelectedMesh(std::optional<std::uint32_t> meshId);

  [[nodiscard]] std::optional<MeshTransform> meshTransform(std::uint32_t meshId) const;
  void setMeshTransform(std::uint32_t meshId, const MeshTransform& transform);

  void resize(int drawableWidth, int drawableHeight) const;
  void beginFrame(float clearR, float clearG, float clearB) const;
  void renderScene(const CameraState& camera, const SceneLighting& lighting) const;
  void endFrame() const;

  [[nodiscard]] std::uint32_t totalTriangles() const;

private:
  struct GpuMesh;

  SDL_Window* window_ = nullptr;
  unsigned int program_ = 0;
  std::vector<GpuMesh> meshes_;
  std::optional<std::uint32_t> hoveredMeshId_;
  std::optional<std::uint32_t> selectedMeshId_;
  std::uint32_t nextMeshId_ = 1;
};

} // namespace sample::rendering
