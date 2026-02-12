#pragma once

#include <cstdint>

#include "engine/modules/IModule.hpp"
#include "engine/modules/ModuleContract.hpp"

namespace engine::modules {

inline constexpr Version kModuleApiVersion{1, 0, 0};

using CreateModuleFn = IModule* (*)();
using DestroyModuleFn = void (*)(IModule* module);
using QueryModuleApiVersionFn = Version (*)();

} // namespace engine::modules

extern "C" {

engine::modules::IModule* CreateModule();
void DestroyModule(engine::modules::IModule* module);
engine::modules::Version QueryModuleApiVersion();

}
