#include "engine/render/opengl/OpenGlRenderBackend.hpp"

#if __has_include(<glad/glad.h>)
#include <glad/glad.h>
#elif __has_include(<glad/gl.h>)
#include <glad/gl.h>
#else
#error "GLAD headers not found. Provide third_party/glad or a glad package."
#endif

#include <cstddef>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string_view>
#include <unordered_map>
#include <unordered_set>

#include "engine/render/ICommandContext.hpp"
#include "engine/render/IRenderBackend.hpp"
#include "engine/render/IRenderDevice.hpp"

namespace engine::render {
namespace {

class OpenGlCommandContext final : public ICommandContext {
public:
  void beginFrame(const FrameGraphFrameInfo& frameInfo) override {
    currentExtent_ = frameInfo.renderExtent;
    glViewport(0, 0, static_cast<GLint>(currentExtent_.width), static_cast<GLint>(currentExtent_.height));
  }

  void endFrame() override { glFlush(); }

  void bindPipeline(const PipelineHandle pipeline) override { activePipeline_ = pipeline; }
  void bindVertexBuffer(const BufferHandle buffer, const std::uint64_t offset) override {
    (void)offset;
    glBindBuffer(GL_ARRAY_BUFFER, static_cast<GLuint>(buffer.id));
  }

  void bindIndexBuffer(const BufferHandle buffer, const std::uint64_t offset) override {
    (void)offset;
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLuint>(buffer.id));
  }

  void draw(const std::uint32_t vertexCount,
            const std::uint32_t instanceCount,
            const std::uint32_t firstVertex,
            const std::uint32_t firstInstance) override {
    (void)activePipeline_;
    (void)firstInstance;
    if (instanceCount <= 1) {
      glDrawArrays(GL_TRIANGLES, static_cast<GLint>(firstVertex), static_cast<GLsizei>(vertexCount));
      return;
    }

    glDrawArraysInstanced(GL_TRIANGLES,
                          static_cast<GLint>(firstVertex),
                          static_cast<GLsizei>(vertexCount),
                          static_cast<GLsizei>(instanceCount));
  }

  void drawIndexed(const std::uint32_t indexCount,
                   const std::uint32_t instanceCount,
                   const std::uint32_t firstIndex,
                   const std::int32_t vertexOffset,
                   const std::uint32_t firstInstance) override {
    (void)vertexOffset;
    (void)firstInstance;
    const auto* offsetPointer = reinterpret_cast<const void*>(static_cast<std::uintptr_t>(firstIndex * sizeof(std::uint32_t)));
    if (instanceCount <= 1) {
      glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indexCount), GL_UNSIGNED_INT, offsetPointer);
      return;
    }

    glDrawElementsInstanced(GL_TRIANGLES,
                            static_cast<GLsizei>(indexCount),
                            GL_UNSIGNED_INT,
                            offsetPointer,
                            static_cast<GLsizei>(instanceCount));
  }

private:
  platform::Extent2D currentExtent_{};
  PipelineHandle activePipeline_{};
};

class OpenGlRenderDevice final : public IRenderDevice {
public:
  [[nodiscard]] std::unique_ptr<ICommandContext> createCommandContext() override {
    return std::make_unique<OpenGlCommandContext>();
  }

  [[nodiscard]] BufferHandle createBuffer(const BufferCreateInfo& createInfo) override {
    GLuint id = 0;
    glGenBuffers(1, &id);
    glBindBuffer(toGlBufferTarget(createInfo.usage), id);
    glBufferData(toGlBufferTarget(createInfo.usage),
                 static_cast<GLsizeiptr>(createInfo.sizeBytes),
                 nullptr,
                 createInfo.cpuVisible ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);

    BufferHandle handle{nextBufferHandle_++};
    liveBuffers_.insert(handle.id);
    glBufferLookup_[handle.id] = id;
    return handle;
  }

  void destroyBuffer(const BufferHandle handle) override {
    const auto it = glBufferLookup_.find(handle.id);
    if (it == glBufferLookup_.end()) {
      return;
    }

    const GLuint id = it->second;
    glDeleteBuffers(1, &id);
    glBufferLookup_.erase(it);
    liveBuffers_.erase(handle.id);
  }

  [[nodiscard]] TextureHandle createTexture(const TextureCreateInfo& createInfo) override {
    GLuint id = 0;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 toGlInternalFormat(createInfo.format),
                 static_cast<GLsizei>(createInfo.extent.width),
                 static_cast<GLsizei>(createInfo.extent.height),
                 0,
                 toGlFormat(createInfo.format),
                 toGlType(createInfo.format),
                 nullptr);

    TextureHandle handle{nextTextureHandle_++};
    liveTextures_.insert(handle.id);
    glTextureLookup_[handle.id] = id;
    return handle;
  }

  void destroyTexture(const TextureHandle handle) override {
    const auto it = glTextureLookup_.find(handle.id);
    if (it == glTextureLookup_.end()) {
      return;
    }

    const GLuint id = it->second;
    glDeleteTextures(1, &id);
    glTextureLookup_.erase(it);
    liveTextures_.erase(handle.id);
  }

  [[nodiscard]] ShaderHandle createShader(const ShaderCreateInfo& createInfo) override {
    const GLenum glStage = toGlShaderStage(createInfo.stage);
    const GLuint shader = glCreateShader(glStage);
    const auto* source = reinterpret_cast<const GLchar*>(createInfo.byteCode);
    const GLint length = static_cast<GLint>(createInfo.byteCodeSize);
    glShaderSource(shader, 1, &source, &length);
    glCompileShader(shader);

    ShaderHandle handle{nextShaderHandle_++};
    liveShaders_.insert(handle.id);
    glShaderLookup_[handle.id] = shader;
    return handle;
  }

  void destroyShader(const ShaderHandle handle) override {
    const auto it = glShaderLookup_.find(handle.id);
    if (it == glShaderLookup_.end()) {
      return;
    }

    glDeleteShader(it->second);
    glShaderLookup_.erase(it);
    liveShaders_.erase(handle.id);
  }

  [[nodiscard]] PipelineHandle createGraphicsPipeline(const GraphicsPipelineCreateInfo& createInfo) override {
    const auto vertexIt = glShaderLookup_.find(createInfo.vertexShader.id);
    const auto fragmentIt = glShaderLookup_.find(createInfo.fragmentShader.id);
    if (vertexIt == glShaderLookup_.end() || fragmentIt == glShaderLookup_.end()) {
      throw std::runtime_error("OpenGL pipeline creation requires valid vertex and fragment shaders");
    }

    const GLuint program = glCreateProgram();
    glAttachShader(program, vertexIt->second);
    glAttachShader(program, fragmentIt->second);
    glLinkProgram(program);

    PipelineHandle handle{nextPipelineHandle_++};
    livePipelines_.insert(handle.id);
    glPipelineLookup_[handle.id] = program;
    return handle;
  }

  void destroyPipeline(const PipelineHandle handle) override {
    const auto it = glPipelineLookup_.find(handle.id);
    if (it == glPipelineLookup_.end()) {
      return;
    }

    glDeleteProgram(it->second);
    glPipelineLookup_.erase(it);
    livePipelines_.erase(handle.id);
  }

private:
  [[nodiscard]] static GLenum toGlBufferTarget(const BufferUsage usage) {
    switch (usage) {
    case BufferUsage::Vertex:
      return GL_ARRAY_BUFFER;
    case BufferUsage::Index:
      return GL_ELEMENT_ARRAY_BUFFER;
    case BufferUsage::Uniform:
      return GL_UNIFORM_BUFFER;
    case BufferUsage::Storage:
      return GL_SHADER_STORAGE_BUFFER;
    default:
      return GL_ARRAY_BUFFER;
    }
  }

  [[nodiscard]] static GLenum toGlShaderStage(const ShaderStage stage) {
    switch (stage) {
    case ShaderStage::Vertex:
      return GL_VERTEX_SHADER;
    case ShaderStage::Fragment:
      return GL_FRAGMENT_SHADER;
    case ShaderStage::Compute:
      return GL_COMPUTE_SHADER;
    default:
      return GL_VERTEX_SHADER;
    }
  }

  [[nodiscard]] static GLint toGlInternalFormat(const TextureFormat format) {
    switch (format) {
    case TextureFormat::RGBA8:
      return GL_RGBA8;
    case TextureFormat::RGBA16F:
      return GL_RGBA16F;
    case TextureFormat::Depth24Stencil8:
      return GL_DEPTH24_STENCIL8;
    default:
      return GL_RGBA8;
    }
  }

  [[nodiscard]] static GLenum toGlFormat(const TextureFormat format) {
    switch (format) {
    case TextureFormat::Depth24Stencil8:
      return GL_DEPTH_STENCIL;
    case TextureFormat::RGBA8:
    case TextureFormat::RGBA16F:
    default:
      return GL_RGBA;
    }
  }

  [[nodiscard]] static GLenum toGlType(const TextureFormat format) {
    switch (format) {
    case TextureFormat::RGBA16F:
      return GL_HALF_FLOAT;
    case TextureFormat::Depth24Stencil8:
      return GL_UNSIGNED_INT_24_8;
    case TextureFormat::RGBA8:
    default:
      return GL_UNSIGNED_BYTE;
    }
  }

  std::uint32_t nextBufferHandle_ = 1;
  std::uint32_t nextTextureHandle_ = 1;
  std::uint32_t nextShaderHandle_ = 1;
  std::uint32_t nextPipelineHandle_ = 1;

  std::unordered_set<std::uint32_t> liveBuffers_;
  std::unordered_set<std::uint32_t> liveTextures_;
  std::unordered_set<std::uint32_t> liveShaders_;
  std::unordered_set<std::uint32_t> livePipelines_;

  std::unordered_map<std::uint32_t, GLuint> glBufferLookup_;
  std::unordered_map<std::uint32_t, GLuint> glTextureLookup_;
  std::unordered_map<std::uint32_t, GLuint> glShaderLookup_;
  std::unordered_map<std::uint32_t, GLuint> glPipelineLookup_;
};

class OpenGlRenderBackend final : public IRenderBackend {
public:
  [[nodiscard]] std::string_view name() const override { return "OpenGL"; }

  [[nodiscard]] std::unique_ptr<IRenderDevice> createDevice() override {
    return std::make_unique<OpenGlRenderDevice>();
  }
};

} // namespace

std::unique_ptr<IRenderBackend> createOpenGlRenderBackend() {
  return std::make_unique<OpenGlRenderBackend>();
}

} // namespace engine::render
