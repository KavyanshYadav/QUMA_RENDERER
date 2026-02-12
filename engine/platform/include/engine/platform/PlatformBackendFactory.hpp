#pragma once

#include <memory>

namespace engine::platform {

class IPlatformBackend;

enum class PlatformBackendType {
  SDL,
};

[[nodiscard]] std::unique_ptr<IPlatformBackend> createPlatformBackend(PlatformBackendType type = PlatformBackendType::SDL);

} // namespace engine::platform
