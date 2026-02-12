#include "engine/render/RenderBackendFactory.hpp"

#include <algorithm>
#include <cctype>
#include <stdexcept>
#include <string>

#include "engine/render/IRenderBackend.hpp"

#if ENGINE_RENDER_HAS_OPENGL
#include "engine/render/opengl/OpenGlRenderBackend.hpp"
#endif

namespace engine::render {
namespace {

[[nodiscard]] std::string normalize(std::string_view value) {
  std::string normalized{value};
  std::ranges::transform(normalized, normalized.begin(), [](const unsigned char ch) {
    return static_cast<char>(std::tolower(ch));
  });
  return normalized;
}

} // namespace

std::optional<RenderBackendType> parseRenderBackendType(const std::string_view value) {
  const std::string normalized = normalize(value);

  if (normalized == "auto") {
    return RenderBackendType::Auto;
  }
  if (normalized == "opengl" || normalized == "gl") {
    return RenderBackendType::OpenGL;
  }
  if (normalized == "vulkan" || normalized == "vk") {
    return RenderBackendType::Vulkan;
  }
  if (normalized == "directx" || normalized == "d3d") {
    return RenderBackendType::DirectX;
  }
  return std::nullopt;
}

RenderBackendType selectRenderBackendType(const std::optional<RenderBackendType> configValue,
                                          const std::span<const std::string_view> cliArgs,
                                          const RenderBackendType fallback) {
  for (const std::string_view argument : cliArgs) {
    constexpr std::string_view prefix = "--render-backend=";
    if (!argument.starts_with(prefix)) {
      continue;
    }

    const std::string_view cliValue = argument.substr(prefix.size());
    if (const auto parsed = parseRenderBackendType(cliValue); parsed.has_value()) {
      return *parsed;
    }
  }

  if (configValue.has_value()) {
    return *configValue;
  }

  return fallback;
}

std::unique_ptr<IRenderBackend> createRenderBackend(RenderBackendType type) {
  if (type == RenderBackendType::Auto) {
    type = RenderBackendType::OpenGL;
  }

  switch (type) {
  case RenderBackendType::OpenGL:
#if ENGINE_RENDER_HAS_OPENGL
    return createOpenGlRenderBackend();
#else
    throw std::runtime_error("OpenGL backend requested but not available in this build");
#endif
  case RenderBackendType::Vulkan:
    throw std::runtime_error("Vulkan backend requested but not yet implemented");
  case RenderBackendType::DirectX:
    throw std::runtime_error("DirectX backend requested but not yet implemented");
  case RenderBackendType::Auto:
  default:
    throw std::runtime_error("Unknown render backend requested");
  }
}

} // namespace engine::render
