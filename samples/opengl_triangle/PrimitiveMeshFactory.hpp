#pragma once

#include <string_view>

#include "ObjLoader.hpp"

namespace sample::app {

enum class PrimitiveMeshType {
  Backpack,
  Sphere,
  Cone,
};

[[nodiscard]] rendering::MeshData createPrimitiveMesh(PrimitiveMeshType type, std::string_view backpackObj);
[[nodiscard]] const char* primitiveMeshTypeName(PrimitiveMeshType type);

} // namespace sample::app
