#pragma once

#include <memory>

#include "engine/render/RenderTypes.hpp"

namespace engine::render {

class ICommandContext;

class IRenderDevice {
public:
  virtual ~IRenderDevice() = default;

  [[nodiscard]] virtual std::unique_ptr<ICommandContext> createCommandContext() = 0;

  [[nodiscard]] virtual BufferHandle createBuffer(const BufferCreateInfo& createInfo) = 0;
  virtual void destroyBuffer(BufferHandle handle) = 0;

  [[nodiscard]] virtual TextureHandle createTexture(const TextureCreateInfo& createInfo) = 0;
  virtual void destroyTexture(TextureHandle handle) = 0;

  [[nodiscard]] virtual ShaderHandle createShader(const ShaderCreateInfo& createInfo) = 0;
  virtual void destroyShader(ShaderHandle handle) = 0;

  [[nodiscard]] virtual PipelineHandle createGraphicsPipeline(const GraphicsPipelineCreateInfo& createInfo) = 0;
  virtual void destroyPipeline(PipelineHandle handle) = 0;
};

} // namespace engine::render
