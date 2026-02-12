#pragma once

#include <string_view>

#include "engine/render/RenderTypes.hpp"

namespace engine::render {

class IRenderDevice;
class ICommandContext;

class IFrameGraphHook {
public:
  virtual ~IFrameGraphHook() = default;

  [[nodiscard]] virtual std::string_view passName() const = 0;
  virtual void setup(IRenderDevice& device) = 0;
  virtual void execute(ICommandContext& commandContext, const FrameGraphFrameInfo& frameInfo) = 0;
  virtual void teardown(IRenderDevice& device) = 0;
};

} // namespace engine::render
