#pragma once

#include <cstddef>
#include <cstdint>

#include "engine/platform/PlatformTypes.hpp"

namespace engine::render {

enum class RenderBackendType {
  Auto,
  OpenGL,
  Vulkan,
  DirectX,
};

enum class BufferUsage {
  Vertex,
  Index,
  Uniform,
  Storage,
};

enum class TextureDimension {
  Texture2D,
  Texture3D,
  TextureCube,
};

enum class TextureFormat {
  RGBA8,
  RGBA16F,
  Depth24Stencil8,
};

enum class ShaderStage {
  Vertex,
  Fragment,
  Compute,
};

enum class PrimitiveTopology {
  TriangleList,
  TriangleStrip,
  LineList,
};

struct BufferHandle {
  std::uint32_t id = 0;
};

struct TextureHandle {
  std::uint32_t id = 0;
};

struct ShaderHandle {
  std::uint32_t id = 0;
};

struct PipelineHandle {
  std::uint32_t id = 0;
};

struct BufferCreateInfo {
  std::uint64_t sizeBytes = 0;
  BufferUsage usage = BufferUsage::Vertex;
  bool cpuVisible = false;
};

struct TextureCreateInfo {
  TextureDimension dimension = TextureDimension::Texture2D;
  TextureFormat format = TextureFormat::RGBA8;
  platform::Extent2D extent{};
  std::uint32_t depth = 1;
  std::uint32_t mipLevels = 1;
};

struct ShaderCreateInfo {
  ShaderStage stage = ShaderStage::Vertex;
  const std::byte* byteCode = nullptr;
  std::size_t byteCodeSize = 0;
};

struct GraphicsPipelineCreateInfo {
  ShaderHandle vertexShader{};
  ShaderHandle fragmentShader{};
  PrimitiveTopology topology = PrimitiveTopology::TriangleList;
};

struct FrameGraphFrameInfo {
  std::uint64_t frameIndex = 0;
  platform::Extent2D renderExtent{};
};

} // namespace engine::render
