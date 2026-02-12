#include "engine/devtools/imgui_tools/ImguiToolsSuite.hpp"

#include <utility>

namespace engine::devtools::imgui_tools {

#if ENGINE_ENABLE_DEVTOOLS

void ImguiToolsSuite::refresh() {
  frameDashboard_.frameTimes = metrics_.frameHistory();
  frameDashboard_.memory = metrics_.memoryStats();

  modulePanel_.modules = modules_.modules();

  rendererPanel_.resources = renderer_.resources();
  rendererPanel_.stats = renderer_.drawStats();

  configPanel_.entries = config_.entries();
}

bool ImguiToolsSuite::requestModuleAction(std::string_view moduleId, ModuleAction action) {
  const bool success = modules_.performAction(std::string(moduleId), action);
  modulePanel_.lastStatus = success ? "Module action succeeded" : "Module action failed";
  if (success) {
    modulePanel_.modules = modules_.modules();
  }

  return success;
}

bool ImguiToolsSuite::stageConfigValue(std::string_view key, std::string value) {
  auto& slot = configPanel_.stagedValues[std::string(key)];
  const bool changed = slot != value;
  slot = std::move(value);
  configPanel_.hasUnsavedChanges = true;
  return changed;
}

bool ImguiToolsSuite::persistStagedConfig() {
  for (const auto& [key, value] : configPanel_.stagedValues) {
    if (!config_.setValue(key, value)) {
      configPanel_.lastPersistStatus = "Failed to update '" + key + "'";
      return false;
    }
  }

  if (!config_.persist()) {
    configPanel_.lastPersistStatus = "Failed to persist configuration";
    return false;
  }

  configPanel_.stagedValues.clear();
  configPanel_.hasUnsavedChanges = false;
  configPanel_.lastPersistStatus = "Configuration saved";
  configPanel_.entries = config_.entries();
  return true;
}

bool ImguiToolsSuite::discardStagedConfig() {
  const bool success = config_.reload();
  configPanel_.stagedValues.clear();
  configPanel_.hasUnsavedChanges = false;
  configPanel_.lastPersistStatus = success ? "Configuration reloaded" : "Failed to reload configuration";
  configPanel_.entries = config_.entries();
  return success;
}

#endif

} // namespace engine::devtools::imgui_tools
