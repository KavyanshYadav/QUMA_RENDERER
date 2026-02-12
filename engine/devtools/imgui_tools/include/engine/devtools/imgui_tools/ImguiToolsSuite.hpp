#pragma once

#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "engine/devtools/imgui_tools/ToolServices.hpp"

namespace engine::devtools::imgui_tools {

struct FrameDashboardModel {
  std::vector<FrameTimingSample> frameTimes;
  MemoryStats memory{};
};

struct ModulePanelModel {
  std::vector<ModuleRecord> modules;
  std::string lastStatus;
};

struct RendererDebugModel {
  std::vector<RenderResourceStat> resources;
  DrawStats stats{};
};

struct ConfigPanelModel {
  std::vector<ConfigEntry> entries;
  std::unordered_map<std::string, std::string> stagedValues;
  bool hasUnsavedChanges = false;
  std::string lastPersistStatus;
};

#if ENGINE_ENABLE_DEVTOOLS

class ImguiToolsSuite {
public:
  ImguiToolsSuite(IMetricsService& metrics,
                  IModuleManagerService& modules,
                  IRendererDebugService& renderer,
                  IConfigurationService& config)
      : metrics_(metrics), modules_(modules), renderer_(renderer), config_(config) {}

  void refresh();

  [[nodiscard]] const FrameDashboardModel& frameDashboard() const { return frameDashboard_; }
  [[nodiscard]] const ModulePanelModel& modulePanel() const { return modulePanel_; }
  [[nodiscard]] const RendererDebugModel& rendererPanel() const { return rendererPanel_; }
  [[nodiscard]] const ConfigPanelModel& configPanel() const { return configPanel_; }

  bool requestModuleAction(std::string_view moduleId, ModuleAction action);
  bool stageConfigValue(std::string_view key, std::string value);
  bool persistStagedConfig();
  bool discardStagedConfig();

private:
  IMetricsService& metrics_;
  IModuleManagerService& modules_;
  IRendererDebugService& renderer_;
  IConfigurationService& config_;

  FrameDashboardModel frameDashboard_{};
  ModulePanelModel modulePanel_{};
  RendererDebugModel rendererPanel_{};
  ConfigPanelModel configPanel_{};
};

#else

class ImguiToolsSuite {
public:
  ImguiToolsSuite(IMetricsService&, IModuleManagerService&, IRendererDebugService&, IConfigurationService&) {}

  void refresh() {}

  [[nodiscard]] const FrameDashboardModel& frameDashboard() const { return frameDashboard_; }
  [[nodiscard]] const ModulePanelModel& modulePanel() const { return modulePanel_; }
  [[nodiscard]] const RendererDebugModel& rendererPanel() const { return rendererPanel_; }
  [[nodiscard]] const ConfigPanelModel& configPanel() const { return configPanel_; }

  bool requestModuleAction(std::string_view, ModuleAction) { return false; }
  bool stageConfigValue(std::string_view, std::string) { return false; }
  bool persistStagedConfig() { return false; }
  bool discardStagedConfig() { return false; }

private:
  FrameDashboardModel frameDashboard_{};
  ModulePanelModel modulePanel_{};
  RendererDebugModel rendererPanel_{};
  ConfigPanelModel configPanel_{};
};

#endif

} // namespace engine::devtools::imgui_tools
