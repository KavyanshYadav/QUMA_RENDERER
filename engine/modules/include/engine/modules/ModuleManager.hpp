#pragma once

#include <queue>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "engine/modules/ModuleContract.hpp"

namespace engine::modules {

struct ValidationResult {
  bool ok = true;
  std::vector<std::string> errors;
};

class ModuleManager {
public:
  explicit ModuleManager(Version supportedApiVersion)
      : supportedApiVersion_(supportedApiVersion) {}

  [[nodiscard]] ValidationResult validate(const std::vector<ModuleDescriptor>& modules) const;
  [[nodiscard]] std::vector<std::string> startupOrder(const std::vector<ModuleDescriptor>& modules) const;
  [[nodiscard]] bool canHotSwap(const ModuleDescriptor& module) const {
    return module.swapPolicy == SwapPolicy::RuntimeSwappable;
  }

private:
  Version supportedApiVersion_;
};

inline ValidationResult ModuleManager::validate(const std::vector<ModuleDescriptor>& modules) const {
  ValidationResult result{};
  std::unordered_map<std::string, const ModuleDescriptor*> moduleById;

  for (const auto& module : modules) {
    auto [it, inserted] = moduleById.emplace(module.id, &module);
    if (!inserted) {
      result.ok = false;
      result.errors.emplace_back("Duplicate module id detected: " + module.id);
    }

    if (!module.requiredApiVersion.isCompatibleWith(supportedApiVersion_)) {
      result.ok = false;
      std::ostringstream message;
      message << "Module '" << module.id << "' requires incompatible API version "
              << module.requiredApiVersion.major << '.' << module.requiredApiVersion.minor << '.'
              << module.requiredApiVersion.patch;
      result.errors.emplace_back(message.str());
    }
  }

  for (const auto& module : modules) {
    for (const auto& dependency : module.dependencies) {
      if (!moduleById.contains(dependency)) {
        result.ok = false;
        result.errors.emplace_back("Module '" + module.id + "' is missing dependency '" + dependency + "'");
      }
    }

    for (const auto& conflict : module.conflicts) {
      if (moduleById.contains(conflict)) {
        result.ok = false;
        result.errors.emplace_back("Module '" + module.id + "' conflicts with loaded module '" + conflict + "'");
      }
    }
  }

  const auto order = startupOrder(modules);
  if (order.size() != modules.size()) {
    result.ok = false;
    result.errors.emplace_back("Module dependency cycle detected");
  }

  return result;
}

inline std::vector<std::string> ModuleManager::startupOrder(const std::vector<ModuleDescriptor>& modules) const {
  std::unordered_map<std::string, std::size_t> indegree;
  std::unordered_map<std::string, std::vector<std::string>> edges;

  for (const auto& module : modules) {
    indegree[module.id] = module.dependencies.size();
    for (const auto& dependency : module.dependencies) {
      edges[dependency].push_back(module.id);
    }
  }

  std::queue<std::string> ready;
  for (const auto& [moduleId, degree] : indegree) {
    if (degree == 0) {
      ready.push(moduleId);
    }
  }

  std::vector<std::string> order;
  order.reserve(modules.size());

  while (!ready.empty()) {
    auto moduleId = std::move(ready.front());
    ready.pop();
    order.push_back(moduleId);

    for (const auto& dependent : edges[moduleId]) {
      auto& degree = indegree[dependent];
      if (degree > 0) {
        --degree;
      }
      if (degree == 0) {
        ready.push(dependent);
      }
    }
  }

  return order;
}

} // namespace engine::modules
