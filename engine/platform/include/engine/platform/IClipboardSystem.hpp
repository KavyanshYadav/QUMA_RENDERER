#pragma once

#include <string>

namespace engine::platform {

class IClipboardSystem {
public:
  virtual ~IClipboardSystem() = default;

  [[nodiscard]] virtual bool hasText() const = 0;
  [[nodiscard]] virtual std::string text() const = 0;
  virtual bool setText(const std::string& value) = 0;
};

} // namespace engine::platform
