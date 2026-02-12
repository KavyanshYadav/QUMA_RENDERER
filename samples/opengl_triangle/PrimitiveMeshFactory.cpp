#include "PrimitiveMeshFactory.hpp"

#include <cmath>
#include <vector>

namespace sample::app {
namespace {

[[nodiscard]] rendering::Vertex makeVertex(const float px,
                                           const float py,
                                           const float pz,
                                           const float nx,
                                           const float ny,
                                           const float nz,
                                           const float u,
                                           const float v) {
  rendering::Vertex vertex{};
  vertex.position[0] = px;
  vertex.position[1] = py;
  vertex.position[2] = pz;
  vertex.normal[0] = nx;
  vertex.normal[1] = ny;
  vertex.normal[2] = nz;
  vertex.uv[0] = u;
  vertex.uv[1] = v;
  return vertex;
}

[[nodiscard]] rendering::MeshData createSphereMesh() {
  rendering::MeshData mesh{};
  constexpr int kStacks = 10;
  constexpr int kSlices = 16;
  constexpr float kRadius = 0.8f;

  for (int stack = 0; stack <= kStacks; ++stack) {
    const float v = static_cast<float>(stack) / static_cast<float>(kStacks);
    const float phi = v * 3.1415926535f;
    const float y = std::cos(phi);
    const float r = std::sin(phi);

    for (int slice = 0; slice <= kSlices; ++slice) {
      const float u = static_cast<float>(slice) / static_cast<float>(kSlices);
      const float theta = u * 6.283185307f;
      const float x = r * std::cos(theta);
      const float z = r * std::sin(theta);
      mesh.vertices.push_back(makeVertex(x * kRadius, y * kRadius, z * kRadius, x, y, z, u, v));
    }
  }

  for (int stack = 0; stack < kStacks; ++stack) {
    for (int slice = 0; slice < kSlices; ++slice) {
      const std::uint32_t rowA = static_cast<std::uint32_t>(stack * (kSlices + 1));
      const std::uint32_t rowB = static_cast<std::uint32_t>((stack + 1) * (kSlices + 1));
      const std::uint32_t i0 = rowA + static_cast<std::uint32_t>(slice);
      const std::uint32_t i1 = i0 + 1;
      const std::uint32_t i2 = rowB + static_cast<std::uint32_t>(slice);
      const std::uint32_t i3 = i2 + 1;
      mesh.indices.insert(mesh.indices.end(), {i0, i2, i1, i1, i2, i3});
    }
  }

  mesh.material.name = "sphere";
  mesh.material.baseColor[0] = 0.32f;
  mesh.material.baseColor[1] = 0.55f;
  mesh.material.baseColor[2] = 0.85f;
  mesh.material.roughness = 0.45f;
  return mesh;
}

[[nodiscard]] rendering::MeshData createConeMesh() {
  rendering::MeshData mesh{};
  constexpr int kSegments = 20;
  constexpr float kRadius = 0.75f;
  constexpr float kHalfHeight = 0.9f;

  const rendering::Vertex apex = makeVertex(0.0f, kHalfHeight, 0.0f, 0.0f, 1.0f, 0.0f, 0.5f, 1.0f);
  const rendering::Vertex baseCenter = makeVertex(0.0f, -kHalfHeight, 0.0f, 0.0f, -1.0f, 0.0f, 0.5f, 0.5f);

  mesh.vertices.push_back(apex);
  mesh.vertices.push_back(baseCenter);

  for (int i = 0; i <= kSegments; ++i) {
    const float t = static_cast<float>(i) / static_cast<float>(kSegments);
    const float ang = t * 6.283185307f;
    const float x = std::cos(ang) * kRadius;
    const float z = std::sin(ang) * kRadius;

    const float nx = x;
    const float nz = z;
    const float ny = kRadius / (2.0f * kHalfHeight);

    mesh.vertices.push_back(makeVertex(x, -kHalfHeight, z, nx, ny, nz, t, 0.0f));
  }

  for (int i = 0; i < kSegments; ++i) {
    const std::uint32_t ring0 = 2U + static_cast<std::uint32_t>(i);
    const std::uint32_t ring1 = ring0 + 1U;
    mesh.indices.insert(mesh.indices.end(), {0U, ring0, ring1});
    mesh.indices.insert(mesh.indices.end(), {1U, ring1, ring0});
  }

  mesh.material.name = "cone";
  mesh.material.baseColor[0] = 0.78f;
  mesh.material.baseColor[1] = 0.42f;
  mesh.material.baseColor[2] = 0.28f;
  mesh.material.roughness = 0.62f;
  return mesh;
}

} // namespace

rendering::MeshData createPrimitiveMesh(const PrimitiveMeshType type, const std::string_view backpackObj) {
  if (type == PrimitiveMeshType::Sphere) {
    return createSphereMesh();
  }
  if (type == PrimitiveMeshType::Cone) {
    return createConeMesh();
  }
  auto mesh = rendering::loadObjFromString(backpackObj);
  mesh.material.name = "backpack";
  mesh.material.baseColor[0] = 0.45f;
  mesh.material.baseColor[1] = 0.62f;
  mesh.material.baseColor[2] = 0.30f;
  mesh.material.metallic = 0.1f;
  mesh.material.roughness = 0.68f;
  return mesh;
}

const char* primitiveMeshTypeName(const PrimitiveMeshType type) {
  switch (type) {
  case PrimitiveMeshType::Backpack:
    return "Backpack";
  case PrimitiveMeshType::Sphere:
    return "Sphere";
  case PrimitiveMeshType::Cone:
    return "Cone";
  default:
    return "Unknown";
  }
}

} // namespace sample::app
