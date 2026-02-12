#pragma once

#include <cstdint>

#include "engine/render/RenderTypes.hpp"

namespace engine::render {

class ICommandContext {
public:
  virtual ~ICommandContext() = default;

  virtual void beginFrame(const FrameGraphFrameInfo& frameInfo) = 0;
  virtual void endFrame() = 0;

  virtual void bindPipeline(PipelineHandle pipeline) = 0;
  virtual void bindVertexBuffer(BufferHandle buffer, std::uint64_t offset = 0) = 0;
  virtual void bindIndexBuffer(BufferHandle buffer, std::uint64_t offset = 0) = 0;
  virtual void draw(std::uint32_t vertexCount,
                    std::uint32_t instanceCount = 1,
                    std::uint32_t firstVertex = 0,
                    std::uint32_t firstInstance = 0) = 0;
  virtual void drawIndexed(std::uint32_t indexCount,
                           std::uint32_t instanceCount = 1,
                           std::uint32_t firstIndex = 0,
                           std::int32_t vertexOffset = 0,
                           std::uint32_t firstInstance = 0) = 0;
};

} // namespace engine::render
