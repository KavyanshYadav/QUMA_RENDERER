#pragma once

#include <memory>

namespace engine::render {

class IRenderBackend;

[[nodiscard]] std::unique_ptr<IRenderBackend> createOpenGlRenderBackend();

} // namespace engine::render
