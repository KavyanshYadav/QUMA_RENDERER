#pragma once

#include <cstdint>

#include "engine/platform/PlatformTypes.hpp"

namespace engine::platform {

class IInputSystem {
public:
  virtual ~IInputSystem() = default;

  [[nodiscard]] virtual KeyState keyState(std::uint32_t keyCode) const = 0;
  [[nodiscard]] virtual KeyState pointerButtonState(std::uint8_t button) const = 0;
  [[nodiscard]] virtual PointerEvent pointer() const = 0;
};

} // namespace engine::platform
