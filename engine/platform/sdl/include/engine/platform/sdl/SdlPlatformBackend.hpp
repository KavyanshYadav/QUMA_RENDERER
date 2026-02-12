#pragma once

#include <memory>

namespace engine::platform {

class IPlatformBackend;

[[nodiscard]] std::unique_ptr<IPlatformBackend> createSdlPlatformBackend();

} // namespace engine::platform
