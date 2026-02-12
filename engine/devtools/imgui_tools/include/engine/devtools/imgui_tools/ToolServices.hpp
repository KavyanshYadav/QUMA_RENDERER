#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace engine::devtools::imgui_tools {

struct FrameTimingSample {
  double cpuFrameMs = 0.0;
  double gpuFrameMs = 0.0;
};

struct MemoryStats {
  std::size_t residentBytes = 0;
  std::size_t virtualBytes = 0;
  std::size_t allocatedBytes = 0;
  std::size_t budgetBytes = 0;
};

class IMetricsService {
public:
  virtual ~IMetricsService() = default;

  [[nodiscard]] virtual std::vector<FrameTimingSample> frameHistory() const = 0;
  [[nodiscard]] virtual MemoryStats memoryStats() const = 0;
};

struct ModuleRecord {
  std::string id;
  std::string state;
  bool hotReloadSupported = false;
};

enum class ModuleAction : std::uint8_t {
  Load,
  Unload,
  Reload,
};

class IModuleManagerService {
public:
  virtual ~IModuleManagerService() = default;

  [[nodiscard]] virtual std::vector<ModuleRecord> modules() const = 0;
  virtual bool performAction(const std::string& moduleId, ModuleAction action) = 0;
};

struct RenderResourceStat {
  std::string name;
  std::string type;
  std::size_t bytes = 0;
  std::uint32_t references = 0;
};

struct DrawStats {
  std::uint32_t drawCalls = 0;
  std::uint32_t triangles = 0;
  std::uint32_t pipelinesBound = 0;
};

class IRendererDebugService {
public:
  virtual ~IRendererDebugService() = default;

  [[nodiscard]] virtual std::vector<RenderResourceStat> resources() const = 0;
  [[nodiscard]] virtual DrawStats drawStats() const = 0;
};

struct ConfigEntry {
  std::string key;
  std::string value;
  std::string description;
};

class IConfigurationService {
public:
  virtual ~IConfigurationService() = default;

  [[nodiscard]] virtual std::vector<ConfigEntry> entries() const = 0;
  virtual bool setValue(const std::string& key, const std::string& value) = 0;
  virtual bool persist() = 0;
  virtual bool reload() = 0;
};

} // namespace engine::devtools::imgui_tools
