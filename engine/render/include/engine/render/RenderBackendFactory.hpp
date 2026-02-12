#pragma once

#include <memory>
#include <optional>
#include <span>
#include <string_view>

#include "engine/render/RenderTypes.hpp"

namespace engine::render {

class IRenderBackend;

[[nodiscard]] std::optional<RenderBackendType> parseRenderBackendType(std::string_view value);
[[nodiscard]] RenderBackendType selectRenderBackendType(std::optional<RenderBackendType> configValue,
                                                        std::span<const std::string_view> cliArgs,
                                                        RenderBackendType fallback = RenderBackendType::OpenGL);
[[nodiscard]] std::unique_ptr<IRenderBackend> createRenderBackend(RenderBackendType type = RenderBackendType::Auto);

} // namespace engine::render
