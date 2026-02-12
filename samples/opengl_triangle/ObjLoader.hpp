#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace sample::rendering {

struct Vertex {
  float position[3]{};
  float normal[3]{};
  float uv[2]{};
};

struct PbrMaterial {
  std::string name{"default"};
  float baseColor[3]{1.0f, 1.0f, 1.0f};
  float metallic = 0.0f;
  float roughness = 0.7f;
  float ambientOcclusion = 1.0f;
};

struct MeshData {
  std::vector<Vertex> vertices;
  std::vector<std::uint32_t> indices;
  PbrMaterial material;
};

[[nodiscard]] MeshData loadObjFromString(std::string_view objSource);

} // namespace sample::rendering
