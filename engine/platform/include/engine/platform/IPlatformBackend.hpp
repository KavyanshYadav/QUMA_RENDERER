#pragma once

#include "engine/platform/IClipboardSystem.hpp"
#include "engine/platform/IInputSystem.hpp"
#include "engine/platform/IMonitorSystem.hpp"
#include "engine/platform/IWindowSystem.hpp"

namespace engine::platform {

class IPlatformBackend {
public:
  virtual ~IPlatformBackend() = default;

  [[nodiscard]] virtual IWindowSystem& windowSystem() = 0;
  [[nodiscard]] virtual IInputSystem& inputSystem() = 0;
  [[nodiscard]] virtual IMonitorSystem& monitorSystem() = 0;
  [[nodiscard]] virtual IClipboardSystem* clipboardSystem() = 0;
};

} // namespace engine::platform
