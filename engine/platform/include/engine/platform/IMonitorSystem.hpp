#pragma once

#include <vector>

#include "engine/platform/PlatformTypes.hpp"

namespace engine::platform {

class IMonitorSystem {
public:
  virtual ~IMonitorSystem() = default;

  [[nodiscard]] virtual std::vector<MonitorInfo> monitors() const = 0;
  [[nodiscard]] virtual MonitorId primaryMonitor() const = 0;
};

} // namespace engine::platform
