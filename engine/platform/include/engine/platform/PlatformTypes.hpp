#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace engine::platform {

using WindowId = std::uint64_t;
using MonitorId = std::uint32_t;

struct Extent2D {
  std::uint32_t width = 0;
  std::uint32_t height = 0;
};

struct WindowCreateInfo {
  std::string title;
  Extent2D size{1280, 720};
  bool resizable = true;
  bool highDpi = true;
};

struct MonitorInfo {
  MonitorId id = 0;
  std::string name;
  Extent2D nativeResolution{};
  std::int32_t refreshRateHz = 0;
  bool isPrimary = false;
};

enum class KeyState {
  Released,
  Pressed,
};

struct KeyboardEvent {
  std::uint32_t keyCode = 0;
  KeyState state = KeyState::Released;
  bool repeated = false;
};

struct PointerEvent {
  std::int32_t x = 0;
  std::int32_t y = 0;
  std::uint8_t button = 0;
  KeyState state = KeyState::Released;
};

enum class PlatformEventType {
  None,
  QuitRequested,
  WindowClosed,
  WindowResized,
  Keyboard,
  Pointer,
};

struct PlatformEvent {
  PlatformEventType type = PlatformEventType::None;
  WindowId windowId = 0;
  Extent2D resizedExtent{};
  KeyboardEvent keyboard{};
  PointerEvent pointer{};
};

using PlatformEventQueue = std::vector<PlatformEvent>;

} // namespace engine::platform
