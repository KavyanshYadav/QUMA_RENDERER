#include "engine/platform/PlatformBackendFactory.hpp"

#include <stdexcept>

#include "engine/platform/IPlatformBackend.hpp"

#if ENGINE_PLATFORM_HAS_SDL
#include "engine/platform/sdl/SdlPlatformBackend.hpp"
#endif

namespace engine::platform {

std::unique_ptr<IPlatformBackend> createPlatformBackend(const PlatformBackendType type) {
  switch (type) {
  case PlatformBackendType::SDL:
#if ENGINE_PLATFORM_HAS_SDL
    return createSdlPlatformBackend();
#else
    throw std::runtime_error("SDL backend requested but not available in this build");
#endif
  default:
    throw std::runtime_error("Unknown platform backend requested");
  }
}

} // namespace engine::platform
