#pragma once

namespace engine::modules {

class IModule {
public:
  virtual ~IModule() = default;

  virtual void onLoad() = 0;
  virtual void onStart() = 0;
  virtual void onStop() = 0;
  virtual void onUnload() = 0;
};

} // namespace engine::modules
