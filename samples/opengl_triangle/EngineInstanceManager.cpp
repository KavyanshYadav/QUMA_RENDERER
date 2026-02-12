#include "EngineInstanceManager.hpp"

#include <array>

namespace sample::app {
namespace {

[[nodiscard]] std::vector<EngineInstanceManager::ModuleRuntimeState> defaultModules(const engine::modules::Version& apiVersion) {
  return {
      EngineInstanceManager::ModuleRuntimeState{
          .descriptor = engine::modules::ModuleDescriptor{
              .id = "engine.render.mesh",
              .category = "render",
              .moduleVersion = {1, 0, 0},
              .requiredApiVersion = apiVersion,
              .swapPolicy = engine::modules::SwapPolicy::RuntimeSwappable,
              .dependencies = {},
              .conflicts = {}},
          .hotSwapGeneration = 0,
          .enabled = true},
      EngineInstanceManager::ModuleRuntimeState{
          .descriptor = engine::modules::ModuleDescriptor{
              .id = "engine.lighting.basic",
              .category = "render",
              .moduleVersion = {1, 0, 0},
              .requiredApiVersion = apiVersion,
              .swapPolicy = engine::modules::SwapPolicy::RuntimeSwappable,
              .dependencies = {"engine.render.mesh"},
              .conflicts = {}},
          .hotSwapGeneration = 0,
          .enabled = true},
      EngineInstanceManager::ModuleRuntimeState{
          .descriptor = engine::modules::ModuleDescriptor{
              .id = "engine.modules.hot_reload",
              .category = "runtime",
              .moduleVersion = {0, 1, 0},
              .requiredApiVersion = apiVersion,
              .swapPolicy = engine::modules::SwapPolicy::RuntimeSwappable,
              .dependencies = {},
              .conflicts = {}},
          .hotSwapGeneration = 0,
          .enabled = true},
  };
}

[[nodiscard]] std::array<float, 3> profileTint(const std::string& profile) {
  if (profile == "Editor") {
    return {0.45f, 0.62f, 0.30f};
  }
  if (profile == "Game") {
    return {0.65f, 0.44f, 0.24f};
  }
  return {0.26f, 0.48f, 0.77f};
}

} // namespace

EngineInstanceManager::EngineInstanceManager(rendering::MeshRenderEngine& renderer,
                                             const rendering::MeshData& baseMesh,
                                             const engine::modules::Version& apiVersion)
    : renderer_(renderer),
      baseMesh_(baseMesh),
      apiVersion_(apiVersion) {}

void EngineInstanceManager::createInstance(std::string name, std::string config, std::string profile) {
  rendering::MeshData mesh = baseMesh_;
  const auto tint = profileTint(profile);
  mesh.material.baseColor[0] = tint[0];
  mesh.material.baseColor[1] = tint[1];
  mesh.material.baseColor[2] = tint[2];

  const float offsetX = static_cast<float>(instances_.size()) * 2.3f;
  const std::uint32_t meshId = renderer_.addMeshInstance(rendering::MeshRenderEngine::MeshInstanceCreateInfo{
      .mesh = mesh,
      .position = {offsetX, 0.0f, 0.0f},
      .rotationYRadians = 0.0f,
      .scale = 1.0f});

  EngineInstanceRuntime runtime{};
  runtime.summary = EngineInstanceSummary{
      .instanceId = nextInstanceId_++,
      .name = std::move(name),
      .config = std::move(config),
      .profile = std::move(profile),
      .running = true};
  runtime.meshId = meshId;
  runtime.modules = defaultModules(apiVersion_);

  instances_.push_back(std::move(runtime));
}

std::vector<EngineInstanceManager::EngineInstanceRuntime>& EngineInstanceManager::instances() {
  return instances_;
}

const std::vector<EngineInstanceManager::EngineInstanceRuntime>& EngineInstanceManager::instances() const {
  return instances_;
}

std::uint32_t EngineInstanceManager::totalRunningInstances() const {
  std::uint32_t running = 0;
  for (const auto& instance : instances_) {
    if (instance.summary.running) {
      ++running;
    }
  }
  return running;
}

} // namespace sample::app
