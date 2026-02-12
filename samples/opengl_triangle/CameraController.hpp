#pragma once

#include <SDL.h>

#include "MeshRenderEngine.hpp"

namespace sample::app {

class CameraController {
public:
  void updateFromInput(float deltaSeconds, const Uint8* keyboardState, bool allowMouseLook);
  void handleMouseMotion(const SDL_MouseMotionEvent& motion, bool allowMouseLook);
  void setMouseLookActive(bool active);

  [[nodiscard]] const rendering::CameraState& camera() const;

private:
  rendering::CameraState camera_{};
  float yawDegrees_ = -90.0f;
  float pitchDegrees_ = 0.0f;
  float moveSpeed_ = 4.5f;
  float mouseSensitivity_ = 0.12f;
  bool mouseLookActive_ = false;

  void updateForward();
};

} // namespace sample::app
