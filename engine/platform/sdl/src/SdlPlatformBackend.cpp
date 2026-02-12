#include "engine/platform/sdl/SdlPlatformBackend.hpp"

#include <SDL.h>

#include <stdexcept>
#include <string>
#include <unordered_map>

#include "engine/platform/IPlatformBackend.hpp"

namespace engine::platform {

class SdlPlatformBackend final : public IPlatformBackend,
                                 public IWindowSystem,
                                 public IInputSystem,
                                 public IMonitorSystem,
                                 public IClipboardSystem {
public:
  SdlPlatformBackend() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) {
      throw std::runtime_error(std::string{"SDL_Init failed: "} + SDL_GetError());
    }
  }

  ~SdlPlatformBackend() override {
    for (auto& [windowId, window] : windows_) {
      (void)windowId;
      SDL_DestroyWindow(window);
    }
    windows_.clear();
    sdlWindowToId_.clear();

    SDL_Quit();
  }

  [[nodiscard]] IWindowSystem& windowSystem() override { return *this; }
  [[nodiscard]] IInputSystem& inputSystem() override { return *this; }
  [[nodiscard]] IMonitorSystem& monitorSystem() override { return *this; }
  [[nodiscard]] IClipboardSystem* clipboardSystem() override { return this; }

  [[nodiscard]] WindowId createWindow(const WindowCreateInfo& createInfo) override {
    std::uint32_t flags = SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_OPENGL;
    if (createInfo.resizable) {
      flags |= SDL_WINDOW_RESIZABLE;
    }

    auto* window = SDL_CreateWindow(createInfo.title.c_str(),
                                    SDL_WINDOWPOS_CENTERED,
                                    SDL_WINDOWPOS_CENTERED,
                                    static_cast<int>(createInfo.size.width),
                                    static_cast<int>(createInfo.size.height),
                                    static_cast<std::uint32_t>(createInfo.highDpi ? flags : (flags & ~SDL_WINDOW_ALLOW_HIGHDPI)));
    if (window == nullptr) {
      throw std::runtime_error(std::string{"SDL_CreateWindow failed: "} + SDL_GetError());
    }

    const auto windowId = nextWindowId_++;
    windows_[windowId] = window;
    sdlWindowToId_[SDL_GetWindowID(window)] = windowId;
    shouldClose_[windowId] = false;
    return windowId;
  }

  void destroyWindow(WindowId windowId) override {
    const auto it = windows_.find(windowId);
    if (it == windows_.end()) {
      return;
    }

    sdlWindowToId_.erase(SDL_GetWindowID(it->second));
    shouldClose_.erase(windowId);
    SDL_DestroyWindow(it->second);
    windows_.erase(it);
  }

  [[nodiscard]] std::optional<Extent2D> framebufferExtent(WindowId windowId) const override {
    const auto it = windows_.find(windowId);
    if (it == windows_.end()) {
      return std::nullopt;
    }

    int width = 0;
    int height = 0;
    SDL_GetWindowSize(it->second, &width, &height);
    return Extent2D{static_cast<std::uint32_t>(width), static_cast<std::uint32_t>(height)};
  }

  [[nodiscard]] void* nativeWindowHandle(WindowId windowId) const override {
    const auto it = windows_.find(windowId);
    if (it == windows_.end()) {
      return nullptr;
    }

    return it->second;
  }

  [[nodiscard]] bool shouldClose(WindowId windowId) const override {
    const auto it = shouldClose_.find(windowId);
    return it != shouldClose_.end() ? it->second : true;
  }

  void pollEvents(PlatformEventQueue& eventQueue) override {
    SDL_Event sdlEvent;
    while (SDL_PollEvent(&sdlEvent) != 0) {
      PlatformEvent event{};

      switch (sdlEvent.type) {
      case SDL_QUIT:
        event.type = PlatformEventType::QuitRequested;
        break;
      case SDL_WINDOWEVENT: {
        event.windowId = findWindow(sdlEvent.window.windowID);
        if (sdlEvent.window.event == SDL_WINDOWEVENT_CLOSE) {
          shouldClose_[event.windowId] = true;
          event.type = PlatformEventType::WindowClosed;
        } else if (sdlEvent.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
          event.type = PlatformEventType::WindowResized;
          event.resizedExtent = Extent2D{static_cast<std::uint32_t>(sdlEvent.window.data1),
                                         static_cast<std::uint32_t>(sdlEvent.window.data2)};
        }
      } break;
      case SDL_KEYDOWN:
      case SDL_KEYUP:
        event.type = PlatformEventType::Keyboard;
        event.windowId = findWindow(sdlEvent.key.windowID);
        event.keyboard.keyCode = static_cast<std::uint32_t>(sdlEvent.key.keysym.sym);
        event.keyboard.state = sdlEvent.key.state == SDL_PRESSED ? KeyState::Pressed : KeyState::Released;
        event.keyboard.repeated = sdlEvent.key.repeat != 0;
        keyStates_[event.keyboard.keyCode] = event.keyboard.state;
        break;
      case SDL_MOUSEBUTTONDOWN:
      case SDL_MOUSEBUTTONUP:
        event.type = PlatformEventType::Pointer;
        event.windowId = findWindow(sdlEvent.button.windowID);
        event.pointer.x = sdlEvent.button.x;
        event.pointer.y = sdlEvent.button.y;
        event.pointer.button = sdlEvent.button.button;
        event.pointer.state = sdlEvent.button.state == SDL_PRESSED ? KeyState::Pressed : KeyState::Released;
        pointerButtons_[event.pointer.button] = event.pointer.state;
        pointer_ = event.pointer;
        break;
      case SDL_MOUSEMOTION:
        pointer_.x = sdlEvent.motion.x;
        pointer_.y = sdlEvent.motion.y;
        break;
      default:
        break;
      }

      if (event.type != PlatformEventType::None) {
        eventQueue.push_back(event);
      }
    }
  }

  [[nodiscard]] KeyState keyState(std::uint32_t keyCode) const override {
    const auto it = keyStates_.find(keyCode);
    return it != keyStates_.end() ? it->second : KeyState::Released;
  }

  [[nodiscard]] KeyState pointerButtonState(std::uint8_t button) const override {
    const auto it = pointerButtons_.find(button);
    return it != pointerButtons_.end() ? it->second : KeyState::Released;
  }

  [[nodiscard]] PointerEvent pointer() const override { return pointer_; }

  [[nodiscard]] std::vector<MonitorInfo> monitors() const override {
    std::vector<MonitorInfo> result;

    const int displayCount = SDL_GetNumVideoDisplays();
    if (displayCount <= 0) {
      return result;
    }

    result.reserve(static_cast<std::size_t>(displayCount));

    for (int index = 0; index < displayCount; ++index) {
      SDL_DisplayMode mode;
      SDL_zero(mode);
      SDL_GetCurrentDisplayMode(index, &mode);

      MonitorInfo monitor{};
      monitor.id = static_cast<MonitorId>(index);
      monitor.name = SDL_GetDisplayName(index);
      monitor.nativeResolution = Extent2D{static_cast<std::uint32_t>(mode.w), static_cast<std::uint32_t>(mode.h)};
      monitor.refreshRateHz = mode.refresh_rate;
      monitor.isPrimary = index == 0;
      result.push_back(monitor);
    }

    return result;
  }

  [[nodiscard]] MonitorId primaryMonitor() const override { return 0; }

  [[nodiscard]] bool hasText() const override { return SDL_HasClipboardText() == SDL_TRUE; }

  [[nodiscard]] std::string text() const override {
    char* value = SDL_GetClipboardText();
    if (value == nullptr) {
      return {};
    }

    const std::string result{value};
    SDL_free(value);
    return result;
  }

  bool setText(const std::string& value) override { return SDL_SetClipboardText(value.c_str()) == 0; }

private:
  [[nodiscard]] WindowId findWindow(std::uint32_t sdlWindowId) const {
    const auto it = sdlWindowToId_.find(sdlWindowId);
    return it != sdlWindowToId_.end() ? it->second : 0;
  }

  WindowId nextWindowId_ = 1;
  std::unordered_map<WindowId, SDL_Window*> windows_;
  std::unordered_map<std::uint32_t, WindowId> sdlWindowToId_;
  std::unordered_map<WindowId, bool> shouldClose_;

  std::unordered_map<std::uint32_t, KeyState> keyStates_;
  std::unordered_map<std::uint8_t, KeyState> pointerButtons_;
  PointerEvent pointer_{};
};

std::unique_ptr<IPlatformBackend> createSdlPlatformBackend() {
  return std::make_unique<SdlPlatformBackend>();
}

} // namespace engine::platform
