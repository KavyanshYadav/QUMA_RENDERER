#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "MeshRenderEngine.hpp"
#include "ObjLoader.hpp"
#include "engine/modules/ModuleContract.hpp"

namespace sample::app {

class EngineInstanceManager {
public:
  struct EngineInstanceSummary {
    std::uint32_t instanceId = 0;
    std::string name;
    std::string config;
    std::string profile;
    bool running = true;
  };

  struct ModuleRuntimeState {
    engine::modules::ModuleDescriptor descriptor;
    std::uint32_t hotSwapGeneration = 0;
    bool enabled = true;
  };

  struct EngineInstanceRuntime {
    EngineInstanceSummary summary;
    std::uint32_t meshId = 0;
    std::vector<ModuleRuntimeState> modules;
  };

  EngineInstanceManager(rendering::MeshRenderEngine& renderer,
                        const rendering::MeshData& baseMesh,
                        const engine::modules::Version& apiVersion);

  void createInstance(std::string name, std::string config, std::string profile);
  [[nodiscard]] std::vector<EngineInstanceRuntime>& instances();
  [[nodiscard]] const std::vector<EngineInstanceRuntime>& instances() const;

  [[nodiscard]] std::uint32_t totalRunningInstances() const;

private:
  rendering::MeshRenderEngine& renderer_;
  rendering::MeshData baseMesh_;
  engine::modules::Version apiVersion_{};
  std::uint32_t nextInstanceId_ = 1;
  std::vector<EngineInstanceRuntime> instances_;
};

} // namespace sample::app
