#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace engine::modules {

enum class SwapPolicy : std::uint8_t {
  RuntimeSwappable,
  RestartRequired,
};

struct RuntimeSwappable {
  static constexpr SwapPolicy value = SwapPolicy::RuntimeSwappable;
};

struct RestartRequired {
  static constexpr SwapPolicy value = SwapPolicy::RestartRequired;
};

struct Version {
  std::uint16_t major = 0;
  std::uint16_t minor = 0;
  std::uint16_t patch = 0;

  [[nodiscard]] constexpr bool isCompatibleWith(const Version& expected) const {
    return major == expected.major && minor >= expected.minor;
  }
};

struct ModuleDescriptor {
  std::string id;
  std::string category;
  Version moduleVersion;
  Version requiredApiVersion;
  SwapPolicy swapPolicy = SwapPolicy::RestartRequired;
  std::vector<std::string> dependencies;
  std::vector<std::string> conflicts;

  [[nodiscard]] bool dependsOn(std::string_view moduleId) const;
  [[nodiscard]] bool conflictsWith(std::string_view moduleId) const;
};

inline bool ModuleDescriptor::dependsOn(std::string_view moduleId) const {
  for (const auto& dependency : dependencies) {
    if (dependency == moduleId) {
      return true;
    }
  }

  return false;
}

inline bool ModuleDescriptor::conflictsWith(std::string_view moduleId) const {
  for (const auto& conflict : conflicts) {
    if (conflict == moduleId) {
      return true;
    }
  }

  return false;
}

} // namespace engine::modules
