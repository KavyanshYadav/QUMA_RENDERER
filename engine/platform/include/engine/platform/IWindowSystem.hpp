#pragma once

#include <optional>

#include "engine/platform/PlatformTypes.hpp"

namespace engine::platform {

class IWindowSystem {
public:
  virtual ~IWindowSystem() = default;

  [[nodiscard]] virtual WindowId createWindow(const WindowCreateInfo& createInfo) = 0;
  virtual void destroyWindow(WindowId windowId) = 0;

  [[nodiscard]] virtual std::optional<Extent2D> framebufferExtent(WindowId windowId) const = 0;
  [[nodiscard]] virtual bool shouldClose(WindowId windowId) const = 0;

  virtual void pollEvents(PlatformEventQueue& eventQueue) = 0;
};

} // namespace engine::platform
