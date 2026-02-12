#include "CameraController.hpp"

#include <cmath>

namespace sample::app {
namespace {

struct Vec3 {
  float x = 0.0f;
  float y = 0.0f;
  float z = 0.0f;
};

[[nodiscard]] Vec3 normalize(const Vec3& v) {
  const float len = std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
  if (len <= 0.0001f) {
    return {0.0f, 0.0f, -1.0f};
  }
  return {v.x / len, v.y / len, v.z / len};
}

[[nodiscard]] Vec3 cross(const Vec3& a, const Vec3& b) {
  return {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
}

} // namespace

void CameraController::updateFromInput(const float deltaSeconds, const Uint8* keyboardState, const bool allowMouseLook) {
  if (allowMouseLook && mouseLookActive_) {
    const Vec3 forward{camera_.forward[0], camera_.forward[1], camera_.forward[2]};
    const Vec3 worldUp{0.0f, 1.0f, 0.0f};
    const Vec3 right = normalize(cross(forward, worldUp));

    const float moveAmount = moveSpeed_ * deltaSeconds;
    if (keyboardState[SDL_SCANCODE_W]) {
      camera_.position[0] += forward.x * moveAmount;
      camera_.position[1] += forward.y * moveAmount;
      camera_.position[2] += forward.z * moveAmount;
    }
    if (keyboardState[SDL_SCANCODE_S]) {
      camera_.position[0] -= forward.x * moveAmount;
      camera_.position[1] -= forward.y * moveAmount;
      camera_.position[2] -= forward.z * moveAmount;
    }
    if (keyboardState[SDL_SCANCODE_A]) {
      camera_.position[0] -= right.x * moveAmount;
      camera_.position[1] -= right.y * moveAmount;
      camera_.position[2] -= right.z * moveAmount;
    }
    if (keyboardState[SDL_SCANCODE_D]) {
      camera_.position[0] += right.x * moveAmount;
      camera_.position[1] += right.y * moveAmount;
      camera_.position[2] += right.z * moveAmount;
    }
  }
}

void CameraController::handleMouseMotion(const SDL_MouseMotionEvent& motion, const bool allowMouseLook) {
  if (!allowMouseLook || !mouseLookActive_) {
    return;
  }

  yawDegrees_ += static_cast<float>(motion.xrel) * mouseSensitivity_;
  pitchDegrees_ -= static_cast<float>(motion.yrel) * mouseSensitivity_;

  if (pitchDegrees_ > 89.0f) {
    pitchDegrees_ = 89.0f;
  }
  if (pitchDegrees_ < -89.0f) {
    pitchDegrees_ = -89.0f;
  }

  updateForward();
}

void CameraController::setMouseLookActive(const bool active) {
  mouseLookActive_ = active;
  SDL_SetRelativeMouseMode(active ? SDL_TRUE : SDL_FALSE);
}

const rendering::CameraState& CameraController::camera() const {
  return camera_;
}

void CameraController::updateForward() {
  const float yawRadians = yawDegrees_ * 0.0174532925f;
  const float pitchRadians = pitchDegrees_ * 0.0174532925f;
  const Vec3 forward = normalize({std::cos(yawRadians) * std::cos(pitchRadians),
                                  std::sin(pitchRadians),
                                  std::sin(yawRadians) * std::cos(pitchRadians)});

  camera_.forward[0] = forward.x;
  camera_.forward[1] = forward.y;
  camera_.forward[2] = forward.z;
  camera_.up[0] = 0.0f;
  camera_.up[1] = 1.0f;
  camera_.up[2] = 0.0f;
}

} // namespace sample::app
