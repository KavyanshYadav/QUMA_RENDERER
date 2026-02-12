#pragma once

#include <memory>
#include <string_view>

namespace engine::render {

class IRenderDevice;

class IRenderBackend {
public:
  virtual ~IRenderBackend() = default;

  [[nodiscard]] virtual std::string_view name() const = 0;
  [[nodiscard]] virtual std::unique_ptr<IRenderDevice> createDevice() = 0;
};

} // namespace engine::render
